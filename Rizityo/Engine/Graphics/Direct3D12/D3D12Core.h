#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	namespace Camera
	{
		class D3D12Camera;
	}

	struct D3D12FrameInfo
	{
		const FrameInfo* FrameInfo;
		Camera::D3D12Camera* Camera;
		D3D12_GPU_VIRTUAL_ADDRESS GlobalShaderData;
		uint32 SurfaceWidth{};
		uint32 SurfaceHeight{};
		uint32 FrameIndex;
		float32 DeltaTime;
	};
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

	[[nodiscard]] ID3D12Device* const GetMainDevice();
	[[nodiscard]] DescriptorHeap& GetRTVHeap();
	[[nodiscard]] DescriptorHeap& GetDSVHeap();
	[[nodiscard]] DescriptorHeap& GetUAVHeap();
	[[nodiscard]] DescriptorHeap& GetSRVHeap();
	[[nodiscard]] ConstantBuffer& GetConstantBuffer();
	[[nodiscard]] uint32 GetCurrentFrameIndex();

	void SetDeferredReleasesFlag();

	[[nodiscard]] Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);
	void ResizeSurface(SurfaceID id, uint32 width, uint32 height);
	[[nodiscard]] uint32 GetSurfaceWidth(SurfaceID id);
	[[nodiscard]] uint32 GetSurfaceHeight(SurfaceID id);
	void RenderSurface(SurfaceID id, FrameInfo info);
}