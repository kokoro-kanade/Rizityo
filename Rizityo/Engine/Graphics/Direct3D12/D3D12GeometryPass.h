#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	struct D3D12FrameInfo;
}

namespace Rizityo::Graphics::D3D12::GPass
{
	bool Initialize();
	void Shutdown();

	[[nodiscard]] const D3D12RenderTexture& GetMainBuffer();
	[[nodiscard]] const D3D12DepthBuffer& GetDepthBuffer();

	void SetSize(Math::U32Vector2 size);

	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);
	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);

	void AddTransitionsForDepthPrepass(Helper::D3D12ResourceBarrier& barriers);
	void AddTransitionsForGPass(Helper::D3D12ResourceBarrier& barriers);
	void AddTransitionsForPostProcess(Helper::D3D12ResourceBarrier& barriers);

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList);
}