#include "D3D12Resource.h"
#include "D3D12Core.h"

namespace Rizityo::Graphics::D3D12
{
	// DescriptorHeap

	bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ _Mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		if (_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || _Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}

		Release();

		auto* const device{ Core::GetMainDevice() };
		assert(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = _Type;
		desc.NodeMask = 0;

		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_Heap)));
		if (FAILED(hr))
			return false;

		_FreeHandles = std::move(std::make_unique<uint32[]>(capacity));
		_Capacity = capacity;
		_Size = 0;

		for (uint32 i = 0; i < capacity; i++)
		{
			_FreeHandles[i] = i;
		}
		DEBUG_ONLY(for (uint32 i = 0; i < FrameBufferCount; i++) assert(_DeferredFreeIndices[i].empty()));

		_DescriptorSize = device->GetDescriptorHandleIncrementSize(_Type);
		_CPUStart = _Heap->GetCPUDescriptorHandleForHeapStart();
		_GPUStart = isShaderVisible ? _Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::ProcessDeferredFree(uint32 frameIndex)
	{
		std::lock_guard lock{ _Mutex };
		assert(frameIndex < FrameBufferCount);

		Vector<uint32>& indices{ _DeferredFreeIndices[frameIndex] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				_Size--;
				_FreeHandles[_Size] = index;
			}
			indices.clear();
		}
	}

	void DescriptorHeap::Release()
	{
		assert(!_Size);
		Core::DeferredRelease(_Heap);
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		std::lock_guard lock{ _Mutex };
		assert(_Heap);
		assert(_Size < _Capacity);

		const uint32 index = _FreeHandles[_Size];
		const uint32 offset = index * _DescriptorSize;
		_Size++;

		DescriptorHandle handle;
		handle.CPU.ptr = _CPUStart.ptr + offset;
		if (IsShaderVisible())
		{
			handle.GPU.ptr = _GPUStart.ptr + offset;
		}

		handle.Index = index;
		DEBUG_ONLY(handle._Container = this);

		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle& handle)
	{
		if (!handle.IsValid())
			return;

		std::lock_guard lock{ _Mutex };
		assert(_Heap && _Size);
		assert(handle._Container == this);
		assert(handle.CPU.ptr >= _CPUStart.ptr);
		assert((handle.CPU.ptr - _CPUStart.ptr) % _DescriptorSize == 0);
		assert(handle.Index < _Capacity);
		const uint32 index = (uint32)(handle.CPU.ptr - _CPUStart.ptr) / _DescriptorSize;
		assert(handle.Index == index);

		const uint32 frameIndex = Core::GetCurrentFrameIndex();
		_DeferredFreeIndices[frameIndex].push_back(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}

	// D3D12Buffer
	D3D12Buffer::D3D12Buffer(D3D12BufferInitInfo info, bool isCPU_Accessible)
	{
		assert(!_Buffer && info.Size && info.Alignment);
		_Size = (uint32)Math::AlignSizeUp(info.Size, info.Alignment);
		_Buffer = Helper::CreateBuffer(info.Data, _Size, isCPU_Accessible, info.InitialState, info.Flags,
									   info.Heap, info.AlocationInfo.Offset);
		_GPU_Address = _Buffer->GetGPUVirtualAddress();
		SET_NAME_D3D12_OBJECT_INDEXED(_Buffer, _Size, L"D3D12 Buffer - size");
	}

	void D3D12Buffer::Release()
	{
		Core::DeferredRelease(_Buffer);
		_GPU_Address = 0;
		_Size = 0;
	}

	// Constant Buffer
	ConstantBuffer::ConstantBuffer(D3D12BufferInitInfo info)
		: _Buffer{ info, true }
	{
		SET_NAME_D3D12_OBJECT_INDEXED(Buffer(), Size(), L"Constant Buffer - size");

		D3D12_RANGE range{};
		DXCall(Buffer()->Map(0, &range, (void**)(&_CPU_Address)));
		assert(_CPU_Address);
	}

	uint8* const ConstantBuffer::Allocate(uint32 size)
	{
		std::lock_guard lock{ _Mutex };
		const uint32 alignedSize = (uint32)Helper::AlignSizeForConstantBuffer(size);
		assert(_CPU_Offset + alignedSize <= _Buffer.Size());
		if (_CPU_Offset + alignedSize <= _Buffer.Size())
		{
			uint8* const address = _CPU_Address + _CPU_Offset;
			_CPU_Offset += alignedSize;
			return address;
		}

		return nullptr;
	}

	// D3D12Texture
	D3D12Texture::D3D12Texture(D3D12TextureInitInfo info)
	{
		auto* const device = Core::GetMainDevice();
		assert(device);

		D3D12_CLEAR_VALUE* const clearValue
		{
			(info.Desc &&
			(info.Desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
			 info.Desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			 ? &info.ClearValue : nullptr
		};

		if (info.Resource)
		{
			assert(!info.Heap);
			_Resource = info.Resource;
		}
		else if (info.Heap && info.Desc)
		{
			assert(!info.Resource);
			DXCall(device->CreatePlacedResource(
				info.Heap, info.AllocationInfo.Offset,
				info.Desc, info.InitialState, 
				clearValue, IID_PPV_ARGS(&_Resource))
			);
		}
		else if (info.Desc)
		{
			assert(!info.Resource && !info.Heap);
			DXCall(device->CreateCommittedResource(
				&Helper::HeapProperties.DefaultHeap, D3D12_HEAP_FLAG_NONE,
				info.Desc, info.InitialState,
				clearValue, IID_PPV_ARGS(&_Resource))
			);
		}

		assert(_Resource);
		_SRV = Core::GetSRVHeap().Allocate();
		device->CreateShaderResourceView(_Resource, info.SRVDesc, _SRV.CPU);
	}

	void D3D12Texture::Release()
	{
		Core::GetSRVHeap().Free(_SRV);
		Core::DeferredRelease(_Resource);
	}

	// D3D12RenderTexture
	D3D12RenderTexture::D3D12RenderTexture(D3D12TextureInitInfo info)
		: _Texture{ info }
	{
		_MipCount = Resource()->GetDesc().MipLevels;
		assert(_MipCount && _MipCount <= D3D12Texture::MaxMips);

		assert(info.Desc);
		DescriptorHeap& rtvHeap{ Core::GetRTVHeap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = info.Desc->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto* const device{ Core::GetMainDevice() };
		assert(device);

		for (uint32 i = 0; i < _MipCount; i++)
		{
			_RTV[i] = rtvHeap.Allocate();
			device->CreateRenderTargetView(Resource(), &desc, _RTV[i].CPU);
			desc.Texture2D.MipSlice++;
		}
	}

	void D3D12RenderTexture::Release()
	{
		for (uint32 i = 0; i < _MipCount; i++)
		{
			Core::GetRTVHeap().Free(_RTV[i]);
		}
		_Texture.Release();
		_MipCount = 0;
	}

	// D3D12DepthBuffer
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12TextureInitInfo info)
	{
		assert(info.Desc);
		const DXGI_FORMAT dsvFormat{ info.Desc->Format };

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		if (info.Desc->Format == DXGI_FORMAT_D32_FLOAT)
		{
			info.Desc->Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		assert(!info.SRVDesc && !info.Resource);
		info.SRVDesc = &srvDesc;
		_Texture = D3D12Texture{ info };

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Format = dsvFormat;
		dsvDesc.Texture2D.MipSlice = 0;

		_DSV = Core::GetDSVHeap().Allocate();

		auto* const device{ Core::GetMainDevice() };
		assert(device);
		device->CreateDepthStencilView(Resource(), &dsvDesc, _DSV.CPU);
	}

	void D3D12DepthBuffer::Release()
	{
		Core::GetDSVHeap().Free(_DSV);
		_Texture.Release();
	}
}