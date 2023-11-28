#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	struct D3D12FrameInfo;
}

namespace Rizityo::Graphics::D3D12::Light
{
	bool Initialize();
	void Shutdown();

	Graphics::Light Create(LightInitInfo info);
	void Remove(LightID id, uint64 lightSetKey);
	void SetParameter(LightID id, uint64 lightSetKey, LightParameter::Parameter parameter, const void* const data, uint32 dataSize);
	void GetParameter(LightID id, uint64 lightSetKey, LightParameter::Parameter parameter, OUT void* const data, uint32 dataSize);

	void UpdateLightBuffers(const D3D12FrameInfo& d3d12Info);
	D3D12_GPU_VIRTUAL_ADDRESS GetNonCullableLightBuffer(uint32 frameIndex);
	uint32 GetNonCullableLightCount(uint64 lightSetKey);
}