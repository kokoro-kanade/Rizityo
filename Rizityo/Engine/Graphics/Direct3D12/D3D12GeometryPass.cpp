#include "D3D12GeometryPass.h"
#include "D3D12Core.h"
#include "D3D12Shader.h"

namespace Rizityo::Graphics::D3D12::GPass
{
	namespace
	{
		struct GPassRootParamIndices
		{
			enum : uint32
			{
				RootConstants,
				Count
			};
		};

		constexpr DXGI_FORMAT MainBufferFormat{ DXGI_FORMAT_R16G16B16A16_FLOAT };
		constexpr DXGI_FORMAT DepthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
		constexpr Math::U32Vector2 InitialDimensions{ 100,100 };

		D3D12RenderTexture GPassMainBuffer{};
		D3D12DepthBuffer GPassDepthBuffer{};
		Math::U32Vector2 Dimensions{ InitialDimensions };
		// D3D12_RESOURCE_BARRIER_FLAGS Flags{};

		ID3D12RootSignature* GPassRootSig = nullptr;
		ID3D12PipelineState* GPassPSO = nullptr;

#if _DEBUG
		constexpr float32 ClearValue[4]{ 0.5f, 0.5f, 0.5f, 1.f };
#else
		constexpr float32 ClearValue[4]{};
#endif // _DEBUG

	} // 変数

	namespace
	{
		bool CreateBuffers(Math::U32Vector2 size)
		{
			assert(size.x && size.y);
			GPassMainBuffer.Release();
			GPassDepthBuffer.Release();

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0; // 0は64KB(MSAAでは4MB)を表す
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			desc.Format = MainBufferFormat;
			desc.Height = size.y;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 0; // 全てのミップレベル用のスペースを作る
			desc.SampleDesc = { 1,0 };
			desc.Width = size.x;

			// メインバッファ作成
			{
				D3D12TextureInitInfo info{};
				info.Desc = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				info.ClearValue.Format = desc.Format;
				memcpy(&info.ClearValue.Color, &ClearValue[0], sizeof(ClearValue));
				GPassMainBuffer = D3D12RenderTexture{ info };
			}

			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			desc.Format = DepthBufferFormat;
			desc.MipLevels = 1;

			// デプスバッファ作成
			{
				D3D12TextureInitInfo info{};
				info.Desc = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; // ピクセルシェーダーとコンピュートシェーダーで読み込む
				info.ClearValue.Format = desc.Format;
				info.ClearValue.DepthStencil.Depth = 0.f;
				info.ClearValue.DepthStencil.Stencil = 0;

				GPassDepthBuffer = D3D12DepthBuffer{ info };
			}

			SET_NAME_D3D12_OBJECT(GPassMainBuffer.Resource(), L"GPass Main Buffer");
			SET_NAME_D3D12_OBJECT(GPassDepthBuffer.Resource(), L"GPass Depth Buffer");

			// Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			return GPassMainBuffer.Resource() && GPassDepthBuffer.Resource();
		}

		bool CreateGPassPSOAndRootSignature()
		{
			assert(!GPassRootSig && !GPassPSO);

			 // GPassルートシグネチャー作成
			using grpid = GPassRootParamIndices;
			Helper::D3D12RootParameter parameters[grpid::Count]{};
			parameters[grpid::RootConstants].AsConstants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);
			const Helper::D3D12RootSignatureDesc rootSignature{ &parameters[0], grpid::Count };
			GPassRootSig = rootSignature.Create();
			assert(GPassRootSig);
			SET_NAME_D3D12_OBJECT(GPassRootSig, L"GPass Root Signature");

			// GPass PSO作成
			struct {
				Helper::D3D12PipelineStateSubobjectRootSignature RootSignature{ GPassRootSig };
				Helper::D3D12PipelineStateSubobjectVS VS{ Shader::GetEngineShader(Shader::EngineShader::FullScreenTriangleVS) };
				Helper::D3D12PipelineStateSubobjectPS PS{ Shader::GetEngineShader(Shader::EngineShader::FillColorPS) };
				Helper::D3D12PipelineStateSubobjectPrimitiveTopology PrimitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				Helper::D3D12PipelineStateSubobjectRenderTargetFormats RenderTargetFormats;
				Helper::D3D12PipelineStateSubobjectDepthStencilFormat DepthStencilFormat{ DepthBufferFormat };
				Helper::D3D12PipelineStateSubobjectRasterizer Rasterizer{ Helper::RasterizerState.NoCull };
				Helper::D3D12PipelineStateSubobjectDepthStencil1 DepthStencil{ Helper::DepthState.Disabled };
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = MainBufferFormat;
			stream.RenderTargetFormats = rtfArray;

			GPassPSO = Helper::CreatePipelineState(&stream, sizeof(stream));
			SET_NAME_D3D12_OBJECT(GPassPSO, L"GPass Pipeline Stete Object");

			return GPassRootSig && GPassPSO;
		}
	} // 関数

	bool Initialize()
	{
		return CreateBuffers(InitialDimensions) && CreateGPassPSOAndRootSignature();
	}

	void Shutdown()
	{
		GPassMainBuffer.Release();
		GPassDepthBuffer.Release();
		Dimensions = InitialDimensions;

		Core::Release(GPassRootSig);
		Core::Release(GPassPSO);
	}

	const D3D12RenderTexture& GetMainBuffer()
	{
		return GPassMainBuffer;
	}

	const D3D12DepthBuffer& GetDepthBuffer()
	{
		return GPassDepthBuffer;
	}

	void SetSize(Math::U32Vector2 size)
	{
		Math::U32Vector2& d{ Dimensions };
		if (size.x > d.x || size.y > d.y)
		{
			d = { std::max(size.x, d.x), std::max(size.y, d.y) };
			CreateBuffers(d);
		}
	}

	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info)
	{

	}

	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info)
	{
		cmdList->SetGraphicsRootSignature(GPassRootSig);
		cmdList->SetPipelineState(GPassPSO);

		static uint32 frame = 0;
		struct
		{
			float32 width;
			float32 height;
			uint32 frame;
		} constants{ (float32)info.SurfaceWidth, (float32)info.SurfaceHeight, ++frame };

		using grpid = GPassRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstants(grpid::RootConstants, 3, &constants, 0);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}

	void AddTransitionsForDepthPrepass(Helper::D3D12ResourceBarrier& barriers)
	{
		/*barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);*/
		barriers.Add(GPassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE/*, Flags*/);

		// Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
	}

	void AddTransitionsForGPass(Helper::D3D12ResourceBarrier& barriers)
	{
		barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET/*, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY*/);
		barriers.Add(GPassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void AddTransitionsForPostProcess(Helper::D3D12ResourceBarrier& barriers)
	{
		barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		/*barriers.Add(GPassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);*/
	}

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV() };

		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(0, nullptr, 0, &dsv);
	}

	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ GPassMainBuffer.RTV(0) };
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV() };

		cmdList->ClearRenderTargetView(rtv, ClearValue, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, 0, &dsv);
	}
}