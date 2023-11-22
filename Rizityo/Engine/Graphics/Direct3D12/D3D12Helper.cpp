#include "D3D12Helper.h"
#include "D3D12Core.h"
#include "D3D12Upload.h"

namespace Rizityo::Graphics::D3D12::Helper
{
	namespace
	{

	} // 無名空間

	void TransitionResource(ID3D12GraphicsCommandList* cmdList,
							ID3D12Resource* resource,
							D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
							D3D12_RESOURCE_BARRIER_FLAGS flags /*= D3D12_RESOURCE_BARRIER_FLAG_NONE*/,
							uint32 subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = subresource;

		cmdList->ResourceBarrier(1, &barrier);
	}

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC vrsDesc{};
		vrsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		vrsDesc.Desc_1_1 = desc;

		using namespace Microsoft::WRL;
		HRESULT hr{ S_OK };
		ComPtr<ID3DBlob> signatureBlob{ nullptr };
		ComPtr<ID3DBlob> errorBlob{ nullptr };
		if (FAILED(hr = D3D12SerializeVersionedRootSignature(&vrsDesc, &signatureBlob, &errorBlob)))
		{
			DEBUG_ONLY(const char* errorMsg = errorBlob ? (const char*)errorBlob->GetBufferPointer() : "");
			DEBUG_ONLY(OutputDebugStringA(errorMsg));
			return nullptr;
		}

		ID3D12RootSignature* signature = nullptr;
		DXCall(hr = Core::GetMainDevice()->CreateRootSignature(
			0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature))
		);

		if (FAILED(hr))
		{
			Core::Release(signature);
		}

		return signature;
	}

	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc)
	{
		assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
		ID3D12PipelineState* pso{ nullptr };
		DXCall(Core::GetMainDevice()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
		assert(pso);
		return pso;
	}

	ID3D12PipelineState* CreatePipelineState(void* stream, uint64 stereamSize)
	{
		assert(stream && stereamSize);
		D3D12_PIPELINE_STATE_STREAM_DESC desc{};
		desc.SizeInBytes = stereamSize;
		desc.pPipelineStateSubobjectStream = stream;
		return CreatePipelineState(desc);
	}

	ID3D12Resource* CreateBuffer(const void* data, uint32 bufferSize, bool isCPU_Accessible /* = false */,
								  D3D12_RESOURCE_STATES state /* = D3D12_RESOURCE_STATE_COMMON */,
								  D3D12_RESOURCE_FLAGS flags /* = D3D12_RESOURCE_FLAG_NONE */,
								  ID3D12Heap* heap /* = nullptr */, uint64 heapOffset /* = 0 */)
	{
		assert(bufferSize);

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = bufferSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = { 1,0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = isCPU_Accessible ? D3D12_RESOURCE_FLAG_NONE : flags;

		// バッファはアップロードあるいは定数バッファ/UAVとして使われる
		assert(desc.Flags == D3D12_RESOURCE_FLAG_NONE ||
			desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		ID3D12Resource* resource = nullptr;
		const D3D12_RESOURCE_STATES resourceState
		{ 
			isCPU_Accessible ? D3D12_RESOURCE_STATE_GENERIC_READ : state
		};

		if (heap)
		{
			DXCall(Core::GetMainDevice()->CreatePlacedResource(
				heap, heapOffset, &desc, resourceState,
				nullptr, IID_PPV_ARGS(&resource)));
		}
		else
		{
			DXCall(Core::GetMainDevice()->CreateCommittedResource(
				isCPU_Accessible ? &HeapProperties.UploadHeap : &HeapProperties.DefaultHeap,
				D3D12_HEAP_FLAG_NONE, &desc, resourceState,
				nullptr, IID_PPV_ARGS(&resource)));
		}

		if (data)
		{
			// 後に変更するデータの場合はisCPU_Accessibleをtrueに
			// GPU用に一回アップロードするだけの場合はisCPU_Accessibleをfalseに
			if (isCPU_Accessible)
			{
				// rangeのBeginとEndを0にすることでCPUは読み込みできないことを表している
				const D3D12_RANGE range{};
				void* cpuAddress = nullptr;
				DXCall(resource->Map(0, &range, reinterpret_cast<void**>(&cpuAddress)));
				assert(cpuAddress);

				memcpy(cpuAddress, data, bufferSize);
				resource->Unmap(0, nullptr);
			}
			else
			{
				Upload::D3D12UploadContext context{ bufferSize };
				memcpy(context.CPU_Address(), data, bufferSize);
				context.CommandList()->CopyResource(resource, context.UploadBuffer());
				context.EndUpload();
			}
		}

		assert(resource);

		return resource;
	}
}
