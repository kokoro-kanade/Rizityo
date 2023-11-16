#include "D3D12Helper.h"
#include "D3D12Core.h"

namespace Rizityo::Graphics::D3D12::Helper
{
	namespace
	{

	} // �������

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
}