#include "TestRenderer.h"
#include "../Platform/PlatformTypes.h"
#include "../Platform/Platform.h"
#include "../Graphics/Renderer.h"

#if TEST_RENDERER

using namespace Rizityo;

Graphics::RenderSurface Surfaces[4];

void DestroyRenderSurface(Graphics::RenderSurface& renderSurface);

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed = true;
		for (uint32 i = 0; i < _countof(Surfaces); i++)
		{
			if (Surfaces[i].Window.IsValid())
			{
				if (Surfaces[i].Window.IsClosed())
				{
					DestroyRenderSurface(Surfaces[i]);
				}
				else
				{
					allClosed = false;
				}
			}
			
		}
		if (allClosed)
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	break;
	case WM_SYSCHAR:
		if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
		{
			Platform::Window window{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
			window.SetFullScreen(!window.IsFullScreen());
			return 0;
		}
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void CreateRenderSurface(Graphics::RenderSurface& renderSurface, Platform::WindowInitInfo info)
{
	renderSurface.Window = Platform::CreateMyWindow(&info);
	renderSurface.Surface = Graphics::CreateSurface(renderSurface.Window);
}

void DestroyRenderSurface(Graphics::RenderSurface& renderSurface)
{
	Graphics::RenderSurface tmp{ renderSurface };
	renderSurface = {};
	if (tmp.Surface.IsValid())
	{
		Graphics::RemoveSurface(tmp.Surface.ID());
	}
	if (tmp.Window.IsValid())
	{
		Platform::RemoveMyWindow(tmp.Window.ID());
	}
}

bool EngineTest::Initialize()
{
	bool result = Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12);
	if (!result)
		return result;

	Platform::WindowInitInfo info[]
	{
		{&WinProc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&WinProc, nullptr, L"Render Window 2", 150, 150, 400, 800},
		{&WinProc, nullptr, L"Render Window 3", 200, 200, 400, 800},
		{&WinProc, nullptr, L"Render Window 4", 250, 250, 400, 800}
	};
	static_assert(_countof(info) == _countof(Surfaces));

	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		CreateRenderSurface(Surfaces[i], info[i]);
	}
	return true;
}

void EngineTest::Run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		if (Surfaces[i].Surface.IsValid())
		{
			Surfaces[i].Surface.Render();
		}
	}
}

void EngineTest::Shutdown()
{
	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		DestroyRenderSurface(Surfaces[i]);
	}
	Graphics::Shutdown();
}

#endif // TEST_RENDERER