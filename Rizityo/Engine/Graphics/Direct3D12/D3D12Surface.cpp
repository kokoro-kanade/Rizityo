#include "D3D12Surface.h"
#include "D3D12Core.h"

namespace Rizityo::Graphics::D3D12
{
	namespace
	{
		constexpr DXGI_FORMAT ToNonSRGB(DXGI_FORMAT format)
		{
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			return format;
		}

	} // 無名空間

	void D3D12Surface::Release()
	{
		for (uint32 i = 0; i < BufferCount; i++)
		{
			RenderTargetData& data{ _RenderTargetData[i] };
			Core::Release(data.Resource);
			Core::GetRTVHeap().Free(data.RTV);
		}

		Core::Release(_SwapChain);
	}

	void D3D12Surface::Finalize()
	{
		for (uint32 i = 0; i < BufferCount; i++)
		{
			RenderTargetData& data{ _RenderTargetData[i] };
			assert(!data.Resource);
			DXCall(_SwapChain->GetBuffer(i, IID_PPV_ARGS(&data.Resource)));
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = DefaultBackBufferFormat;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Core::GetMainDevice()->CreateRenderTargetView(data.Resource, &desc, data.RTV.CPU);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(_SwapChain->GetDesc(&desc));
		const uint32 width = desc.BufferDesc.Width;
		const uint32 height = desc.BufferDesc.Height;
		assert(_Window.Width() == width && _Window.Height() == height);

		_Viewport.TopLeftX = 0.f;
		_Viewport.TopLeftY = 0.f;
		_Viewport.Width = (float)width;
		_Viewport.Height = (float)height;
		_Viewport.MinDepth = 0.f;
		_Viewport.MaxDepth = 1.f;

		_ScissorRect = { 0,0,(int32)width, (int32)height };
	}

	void D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue)
	{
		assert(factory && cmdQueue);
		Release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &_AllowTearing, sizeof(uint32))) && _AllowTearing)
		{
			_PresentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = BufferCount;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = _AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Format = ToNonSRGB(DefaultBackBufferFormat);
		desc.Height = _Window.Height();
		desc.Width = _Window.Width();
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd{ static_cast<HWND>(_Window.Handle()) };
		DXCall(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, nullptr, nullptr, &swapChain));
		DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&_SwapChain)));
		Core::Release(swapChain);

		_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

		for (uint32 i = 0; i < BufferCount; i++)
		{
			_RenderTargetData[i].RTV = Core::GetRTVHeap().Allocate();
		}

		Finalize();
	}

	void D3D12Surface::Present() const
	{
		assert(_SwapChain);
		DXCall(_SwapChain->Present(0, _PresentFlags));
		_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12Surface::Resize()
	{
		assert(_SwapChain);
		for (uint32 i = 0; i < BufferCount; i++)
		{
			Core::Release(_RenderTargetData[i].Resource);
		}

		const uint32 flags = _AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0ul;
		DXCall(_SwapChain->ResizeBuffers(BufferCount, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

		Finalize();

		DEBUG_ONLY(OutputDebugString(L"::D3D12サーフェスがリサイズされました\n"));
	}
	
}