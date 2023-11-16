#include "D3D12Core.h"
#include "D3D12Surface.h"
#include "D3D12Shader.h"
#include "D3D12GeometryPass.h"
#include "D3D12PostProcess.h"

using namespace Microsoft::WRL;

namespace Rizityo::Graphics::D3D12::Core
{
	namespace
	{
		class D3D12Command
		{
		public:
			D3D12Command() = default;
			DISABLE_COPY_AND_MOVE(D3D12Command);

			explicit D3D12Command(ID3D12Device* const device, D3D12_COMMAND_LIST_TYPE type)
			{
				HRESULT hr{ S_OK };

				D3D12_COMMAND_QUEUE_DESC desc{};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				desc.NodeMask = 0;
				desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				desc.Type = type;
				DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_CmdQueue)));
				if (FAILED(hr))
					goto _error;

				SET_NAME_D3D12_OBJECT(_CmdQueue,
					type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command Queue");

				for (uint32 i = 0; i < FrameBufferCount; i++)
				{
					CommandFrame& frame{ _CmdFrames[i] };
					DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.CmdAllocator)));
					if (FAILED(hr))
						goto _error;

					SET_NAME_D3D12_OBJECT_INDEXED(frame.CmdAllocator, i,
						type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Allocator" :
						type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Allocator" : L"Command Allocator");
				}

				DXCall(device->CreateCommandList(0, type, _CmdFrames[0].CmdAllocator, nullptr, IID_PPV_ARGS(&_CmdList)));
				if (FAILED(hr))
					goto _error;

				DXCall(_CmdList->Close());
				SET_NAME_D3D12_OBJECT(_CmdList,
					type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command List" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command List" : L"Command List");

				DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_Fence)));
				if (FAILED(hr))
					goto _error;

				SET_NAME_D3D12_OBJECT(_Fence, L"D3D12 Fence");

				_FenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				assert(_FenceEvent);

				return;

			_error:
				Release();
			}

			~D3D12Command()
			{
				assert(!_CmdQueue && !_CmdList && !_Fence);
			}

			// ���݂̃t���[�����Ăяo�����܂ő҂��ăR�}���h���X�g/�A���P�[�^�����Z�b�g
			void BeginFrame()
			{
				CommandFrame& frame{ _CmdFrames[_FrameIndex] };
				frame.Wait(_FenceEvent, _Fence);
				DXCall(frame.CmdAllocator->Reset());
				DXCall(_CmdList->Reset(frame.CmdAllocator, nullptr));
			}

			// �V����fence�l��`����
			void EndFrame(const D3D12Surface& surface)
			{
				DXCall(_CmdList->Close());
				ID3D12CommandList* const cmdLists[]{ _CmdList };
				_CmdQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);

				surface.Present();

				uint64& fenceValue = _FenceValue;
				fenceValue++;
				CommandFrame& frame{ _CmdFrames[_FrameIndex] };
				frame.FenceValue = fenceValue;
				_CmdQueue->Signal(_Fence, _FenceValue);

				_FrameIndex = (_FrameIndex + 1) % FrameBufferCount;
			}

			void Flush()
			{
				for (uint32 i = 0; i < FrameBufferCount; i++)
				{
					_CmdFrames[i].Wait(_FenceEvent, _Fence);
				}
				_FrameIndex = 0;
			}

			void Release()
			{
				Flush();
				Core::Release(_Fence);
				_FenceValue = 0;

				CloseHandle(_FenceEvent);
				_FenceEvent = nullptr;

				Core::Release(_CmdQueue);
				Core::Release(_CmdList);

				for (uint32 i = 0; i < FrameBufferCount; i++)
				{
					_CmdFrames[i].Release();
				}
			}

			constexpr ID3D12CommandQueue* const CommandQueue() const { return _CmdQueue; }
			constexpr ID3D12GraphicsCommandList* const CommandList() const { return _CmdList; }
			constexpr uint32 FrameIndex() const { return _FrameIndex; }

		private:
			struct CommandFrame
			{
				ID3D12CommandAllocator* CmdAllocator{ nullptr };
				uint64 FenceValue = 0;

				void Wait(HANDLE fenceEvent, ID3D12Fence1* fence)
				{
					assert(fence && fenceEvent);
					// �܂�GPU�̓R�}���h���X�g�̎��s���������Ă��Ȃ�
					if (fence->GetCompletedValue() < FenceValue)
					{
						DXCall(fence->SetEventOnCompletion(FenceValue, fenceEvent));
						WaitForSingleObject(fenceEvent, INFINITE);
					}
				}

				void Release()
				{
					Core::Release(CmdAllocator);
					FenceValue = 0;
				}
			};

			ID3D12CommandQueue* _CmdQueue{ nullptr };
			ID3D12GraphicsCommandList* _CmdList{ nullptr };
			ID3D12Fence1* _Fence{ nullptr };
			uint64 _FenceValue = 0;
			HANDLE _FenceEvent{ nullptr };
			CommandFrame _CmdFrames[FrameBufferCount]{};
			uint32 _FrameIndex = 0;

		};

		ID3D12Device* MainDevice{ nullptr };
		IDXGIFactory7* DxgiFactory{ nullptr };
		D3D12Command gfxCommand;
		Utility::FreeList<D3D12Surface> Surfaces;
		Helper::D3D12ResourceBarrier ResourceBarriers{};

		DescriptorHeap RTVDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
		DescriptorHeap DSVDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
		DescriptorHeap UAVDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		DescriptorHeap SRVDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

		Utility::Vector<IUnknown*> DeferredReleases[FrameBufferCount]{};
		uint32 DeferredReleasesFlag[FrameBufferCount]{};
		std::mutex DeferredReleasesMutex{};

		constexpr D3D_FEATURE_LEVEL MinFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };

	} // �ϐ�

	namespace
	{
		bool FailedInit()
		{
			Shutdown();
			return false;
		}

		IDXGIAdapter4* DetermineMainAdapter()
		{
			IDXGIAdapter4* adapter{ nullptr };
			// �O���t�B�b�N�J�[�h���p�t�H�[�}���X���Ɍ��Ă���
			for (uint32 i = 0;
				DxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
				i++)
			{
				// minFeatureLevel���T�|�[�g����ŏ��̃A�_�v�^�[��I��
				if (SUCCEEDED(D3D12CreateDevice(adapter, MinFeatureLevel, __uuidof(ID3D12Device), nullptr)))
				{
					return adapter;
				}
				Release(adapter);
			}

			return nullptr;
		}

		D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter4* adapter)
		{
			constexpr D3D_FEATURE_LEVEL featureLevels[4]
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1,
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
			featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
			featureLevelInfo.pFeatureLevelsRequested = featureLevels;

			ComPtr<ID3D12Device> device;
			DXCall(D3D12CreateDevice(adapter, MinFeatureLevel, IID_PPV_ARGS(&device)));
			DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));
			return featureLevelInfo.MaxSupportedFeatureLevel;
		}

		__declspec(noinline)
		void ProcessDeferredReleases(uint32 frameIndex)
		{
			std::lock_guard lock{ DeferredReleasesMutex };

			DeferredReleasesFlag[frameIndex] = 0;

			RTVDescHeap.ProcessDeferredFree(frameIndex);
			DSVDescHeap.ProcessDeferredFree(frameIndex);
			UAVDescHeap.ProcessDeferredFree(frameIndex);
			SRVDescHeap.ProcessDeferredFree(frameIndex);

			Utility::Vector<IUnknown*>& resources{ DeferredReleases[frameIndex] };
			if (!resources.empty())
			{
				for (auto& resource : resources)
				{
					Release(resource);
				}
				resources.clear();
			}
		}
	} // �֐�

	namespace Internal
	{
		void DeferredRelease(IUnknown* resource)
		{
			const uint32 frameIndex = GetCurrentFrameIndex();
			std::lock_guard lock{ DeferredReleasesMutex };
			DeferredReleases[frameIndex].push_back(resource);
			SetDeferredReleasesFlag();
		}
	}

	bool Initialize()
	{
		if (MainDevice)
			Shutdown();

		uint32 dxgiFactoryFlags = 0;

#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug3> DebugInterface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugInterface))))
			{
				DebugInterface->EnableDebugLayer();
#if 0
#pragma message("WARNING: GPU based validation���L���ɂȂ��Ă��܂��B�������x���Ȃ�\��������܂�")
				DebugInterface->SetEnableGPUBasedValidation(1);
#endif
			}
			else
			{
				OutputDebugStringA("Warning: D3D12 Debug interface�����p�ł��܂���BGraphics Tools optional feature�����̃f�o�C�X�ɃC���X�g�[������Ă��邩�m�F���Ă��������B\n");
			}
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif // _DEBUG

		HRESULT hr{ S_OK };
		DXCall(hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&DxgiFactory)));
		if (FAILED(hr))
			return FailedInit();

		// �ǂ̃O���t�B�b�N�J�[�h��p���邩
		ComPtr<IDXGIAdapter4> mainAdapter;
		mainAdapter.Attach(DetermineMainAdapter());
		if (!mainAdapter)
			return FailedInit();

		D3D_FEATURE_LEVEL maxFeatureLevel{ GetMaxFeatureLevel(mainAdapter.Get()) };
		assert(maxFeatureLevel >= MinFeatureLevel);
		if (maxFeatureLevel < MinFeatureLevel)
			return FailedInit();

		DXCall(hr = D3D12CreateDevice(mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&MainDevice)));
		if (FAILED(hr))
			return FailedInit();

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif // _DEBUG

		bool result = true;
		result &= RTVDescHeap.Initialize(512, false);
		result &= DSVDescHeap.Initialize(512, false);
		result &= UAVDescHeap.Initialize(512, false);
		result &= SRVDescHeap.Initialize(4096, true);

		SET_NAME_D3D12_OBJECT(RTVDescHeap.Heap(), L"RTV Descriptor Heap");
		SET_NAME_D3D12_OBJECT(DSVDescHeap.Heap(), L"DSV Descriptor Heap");
		SET_NAME_D3D12_OBJECT(UAVDescHeap.Heap(), L"UAV Descriptor Heap");
		SET_NAME_D3D12_OBJECT(SRVDescHeap.Heap(), L"SRV Descriptor Heap");

		new(&gfxCommand) D3D12Command(MainDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!gfxCommand.CommandQueue())
			return FailedInit();

		// ���W���[���̏�����
		if (!(Shader::Initialize() && GPass::Initialize() && Post::Initialize()))
			return FailedInit();

		SET_NAME_D3D12_OBJECT(MainDevice, L"Main D3D12 Device");
		
		return true;
	}

	void Shutdown()
	{
		gfxCommand.Release();

		for (uint32 i = 0; i < FrameBufferCount; i++)
		{
			ProcessDeferredReleases(i);
		}

		// ���W���[���̃V���b�g�_�E��
		Post::Shutdown();
		GPass::Shutdown();
		Shader::Shutdown();

		Release(DxgiFactory);

		RTVDescHeap.ProcessDeferredFree(0);
		DSVDescHeap.ProcessDeferredFree(0);
		UAVDescHeap.ProcessDeferredFree(0);
		SRVDescHeap.ProcessDeferredFree(0);

		RTVDescHeap.Release();
		DSVDescHeap.Release();
		UAVDescHeap.Release();
		SRVDescHeap.Release();

		ProcessDeferredReleases(0);

#ifdef _DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> infoQueue;
				DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			}

			ComPtr<ID3D12DebugDevice2> debugDevice;
			DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&debugDevice)));
			Release(MainDevice);
			DXCall(debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
		}
#endif // _DEBUG

		Release(MainDevice);
	}

	ID3D12Device* const GetMainDevice()
	{
		return MainDevice;
	}

	DescriptorHeap& GetRTVHeap()
	{
		return RTVDescHeap;
	}

	DescriptorHeap& GetDSVHeap()
	{
		return DSVDescHeap;	
	}

	DescriptorHeap& GetUAVHeap()
	{
		return UAVDescHeap;	
	}

	DescriptorHeap& GetSRVHeap()
	{
		return SRVDescHeap;		
	}

	uint32 GetCurrentFrameIndex()
	{
		return gfxCommand.FrameIndex();
	}

	void SetDeferredReleasesFlag()
	{
		DeferredReleasesFlag[GetCurrentFrameIndex()] = 1;
	}

	Surface CreateSurface(Platform::Window window)
	{
		SurfaceID id{ Surfaces.Add(window) };
		Surfaces[id].CreateSwapChain(DxgiFactory, gfxCommand.CommandQueue());
		return Surface{ id };
	}

	void RemoveSurface(SurfaceID id)
	{
		gfxCommand.Flush();
		Surfaces.Remove(id);
	}

	void ResizeSurface(SurfaceID id, uint32 width, uint32 height)
	{
		gfxCommand.Flush();
		Surfaces[id].Resize();
	}

	uint32 GetSurfaceWidth(SurfaceID id)
	{
		return Surfaces[id].Width();
	}

	uint32 GetSurfaceHeight(SurfaceID id)
	{
		return Surfaces[id].Height();
	}

	void RenderSurface(SurfaceID id)
	{
		gfxCommand.BeginFrame();
		ID3D12GraphicsCommandList* cmdList{ gfxCommand.CommandList() };

		const uint32 frameIndex = GetCurrentFrameIndex();
		if (DeferredReleasesFlag[frameIndex])
		{
			ProcessDeferredReleases(frameIndex);
		}

		const D3D12Surface& surface{ Surfaces[id] };
		ID3D12Resource* const currentBackBuffer{ surface.BackBuffer() };
		D3D12FrameInfo frameInfo{ surface.Width(), surface.Height() };

		GPass::SetSize({ frameInfo.SurfaceWidth, frameInfo.SurfaceHeight });

		Helper::D3D12ResourceBarrier& barriers{ ResourceBarriers };

		// �R�}���h�̋L�^
		ID3D12DescriptorHeap* const heaps[]{ SRVDescHeap.Heap() };
		cmdList->SetDescriptorHeaps(1, &heaps[0]);

		cmdList->RSSetViewports(1, &surface.Viewport());
		cmdList->RSSetScissorRects(1, &surface.ScissorRect());

		// �f�v�X�v���p�X
		/*barriers.Add(currentBackBuffer,
					 D3D12_RESOURCE_STATE_PRESENT,
					 D3D12_RESOURCE_STATE_RENDER_TARGET,
					 D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);*/
		GPass::AddTransitionsForDepthPrepass(barriers);
		barriers.Apply(cmdList);
		GPass::SetRenderTargetsForDepthPrepass(cmdList);
		GPass::DepthPrepass(cmdList, frameInfo);

		// �W�I���g���E���C�e�B���O�p�X
		GPass::AddTransitionsForGPass(barriers);
		barriers.Apply(cmdList);
		GPass::SetRenderTargetsForGPass(cmdList);
		GPass::Render(cmdList, frameInfo);

		Helper::TransitionResource(cmdList, currentBackBuffer,
								   D3D12_RESOURCE_STATE_PRESENT,
								   D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �|�X�g�v���Z�X
		/*barriers.Add(currentBackBuffer,
					 D3D12_RESOURCE_STATE_PRESENT,
					 D3D12_RESOURCE_STATE_RENDER_TARGET,
					 D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);*/
		GPass::AddTransitionsForPostProcess(barriers);
		barriers.Apply(cmdList);

		Post::PostProcess(cmdList, surface.RTV());

		// �|�X�g�v���Z�X��
		Helper::TransitionResource(cmdList, currentBackBuffer,
								   D3D12_RESOURCE_STATE_RENDER_TARGET,
								   D3D12_RESOURCE_STATE_PRESENT);

		// �R�}���h���s�E���t���[���̂��߂Ƀt�F���X�l�̃C���N�������g
		gfxCommand.EndFrame(surface);
	}
}