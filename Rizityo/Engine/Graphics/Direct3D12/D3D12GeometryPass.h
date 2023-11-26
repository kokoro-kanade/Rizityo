#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	struct D3D12FrameInfo;
}

namespace Rizityo::Graphics::D3D12::GPass
{
	constexpr DXGI_FORMAT MainBufferFormat{ DXGI_FORMAT_R16G16B16A16_FLOAT };
	constexpr DXGI_FORMAT DepthBufferFormat{ DXGI_FORMAT_D32_FLOAT };

	struct OpaqueRootParameter {
		enum Parameter : uint32 {
			GlobalShaderData,
			PositionBuffer,
			ElementBuffer,
			SrvIndices,
			PerObjectData,
			Count
		};
	};

	bool Initialize();
	void Shutdown();

	[[nodiscard]] const D3D12RenderTexture& GetMainBuffer();
	[[nodiscard]] const D3D12DepthBuffer& GetDepthBuffer();

	void SetSize(Math::U32Vector2 size);

	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info);
	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info);

	void AddTransitionsForDepthPrepass(Helper::D3D12ResourceBarrier& barriers);
	void AddTransitionsForGPass(Helper::D3D12ResourceBarrier& barriers);
	void AddTransitionsForPostProcess(Helper::D3D12ResourceBarrier& barriers);

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList);
}