#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	class DescriptorHeap;
}

namespace Rizityo::Graphics::D3D12::Core
{
	bool Initialize();
	void Shutdown();

	template<typename T>
	constexpr void Release(T*& resource)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	namespace Internal
	{
		void DeferredRelease(IUnknown* resource);
	}

	template<typename T>
	constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			Internal::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	ID3D12Device* const GetMainDevice();
	DescriptorHeap& GetRTVHeap();
	DescriptorHeap& GetDSVHeap();
	DescriptorHeap& GetUAVHeap();
	DescriptorHeap& GetSRVHeap();
	DXGI_FORMAT GetDefaultRenderTargetFormat();
	uint32 GetCurrentFrameIndex();
	void SetDeferredReleasesFlag();

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);
	void ResizeSurface(SurfaceID id, uint32 width, uint32 height);
	uint32 GetSurfaceWidth(SurfaceID id);
	uint32 GetSurfaceHeight(SurfaceID id);
	void RenderSurface(SurfaceID id);
}