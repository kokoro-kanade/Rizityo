#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resource.h"

namespace Rizityo::Graphics::D3D12
{
	class D3D12Surface
	{
	public:
		explicit D3D12Surface(Platform::Window window) : _Window{ window }
		{
			assert(_Window.Handle());
		}

		~D3D12Surface()
		{
			Release();
		}

		DISABLE_COPY_AND_MOVE(D3D12Surface);

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format);
		void Present() const;
		void Resize();
		uint32 Width() const { return (uint32)_Viewport.Width; }
		uint32 Height() const { return (uint32)_Viewport.Height; }
		constexpr ID3D12Resource* const BackBuffer() const { return _RenderTargetData[CurrentBackBufferIndex].Resource; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV() const { return _RenderTargetData[CurrentBackBufferIndex].RTV.CPU; }
		const D3D12_VIEWPORT& Viewport() const { return _Viewport; }
		const D3D12_RECT& ScissorRect() const { return _ScissorRect; }

	private:
		struct RenderTargetData
		{
			ID3D12Resource* Resource{ nullptr };
			DescriptorHandle RTV{};
		};

		Platform::Window _Window{};
		IDXGISwapChain4* _SwapChain{};
		RenderTargetData _RenderTargetData[FrameBufferCount]{};
		D3D12_VIEWPORT _Viewport{};
		D3D12_RECT _ScissorRect{};
		mutable uint32 CurrentBackBufferIndex = 0;
		uint32 AllowTearing = 0;
		uint32 PresentFlags = 0;

		void Release();
		void Finalize();
	};
}