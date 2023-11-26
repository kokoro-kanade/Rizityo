#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	struct D3D12FrameInfo;
}

namespace Rizityo::Graphics::D3D12::Post
{
	bool Initialize();
	void Shutdown();

	void PostProcess(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV);
}