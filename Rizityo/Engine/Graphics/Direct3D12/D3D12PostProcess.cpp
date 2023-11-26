#include "D3D12PostProcess.h"
#include "D3D12Core.h"
#include "D3D12Shader.h"
#include "D3D12Surface.h"
#include "D3D12GeometryPass.h"

namespace Rizityo::Graphics::D3D12::Post
{
	namespace
	{

	}

	namespace
	{
		struct PostRootParamIndices
		{
			enum : uint32
			{
				RootConstants,
				Count
			};
		};

		ID3D12RootSignature* PostRootSig = nullptr;
		ID3D12PipelineState* PostPSO = nullptr;

		bool CreatePostPSOAndRootSignature()
		{
			assert(!PostRootSig && !PostPSO);

			// GPassルートシグネチャー作成
			using prmid = PostRootParamIndices;
			Helper::D3D12RootParameter parameters[prmid::Count]{};
			parameters[prmid::RootConstants].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);

			Helper::D3D12RootSignatureDesc rootSignature{ &parameters[0], _countof(parameters) };
			rootSignature.Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			PostRootSig = rootSignature.Create();
			assert(PostRootSig);
			SET_NAME_D3D12_OBJECT(PostRootSig, L"PostProcess Root Signature");

			// GPass PSO作成
			struct {
				Helper::D3D12PipelineStateSubobjectRootSignature RootSignature{ PostRootSig };
				Helper::D3D12PipelineStateSubobjectVS VS{ Shader::GetEngineShader(Shader::EngineShader::FullScreenTriangleVS) };
				Helper::D3D12PipelineStateSubobjectPS PS{ Shader::GetEngineShader(Shader::EngineShader::PostProcessPS) };
				Helper::D3D12PipelineStateSubobjectPrimitiveTopology PrimitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				Helper::D3D12PipelineStateSubobjectRenderTargetFormats RenderTargetFormats;
				Helper::D3D12PipelineStateSubobjectRasterizer Rasterizer{ Helper::RasterizerState.NoCull };
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = D3D12Surface::DefaultBackBufferFormat;
			stream.RenderTargetFormats = rtfArray;

			PostPSO = Helper::CreatePipelineState(&stream, sizeof(stream));
			SET_NAME_D3D12_OBJECT(PostPSO, L"PostProcess Pipeline Stete Object");

			return PostRootSig && PostPSO;
		}
	}

	bool Initialize()
	{
		return CreatePostPSOAndRootSignature();
	}

	void Shutdown()
	{
		Core::Release(PostRootSig);
		Core::Release(PostPSO);
	}

	void PostProcess(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV)
	{
		cmdList->SetGraphicsRootSignature(PostRootSig);
		cmdList->SetPipelineState(PostPSO);

		using prmid = PostRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstant(prmid::RootConstants, GPass::GetMainBuffer().SRV().Index, 0);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}
}