#include "TestRenderer.h"
#include "../Platform/PlatformTypes.h"
#include "../Platform/Platform.h"
#include "../Graphics/Renderer.h"
#include "ShaderCompile.h"

#if TEST_RENDERER

using namespace Rizityo;

Graphics::RenderSurface Surfaces[4];

bool Resized = false;
bool IsRestarting = false;

bool TestInitialize();
void TestShutdown();

void DestroyRenderSurface(Graphics::RenderSurface& renderSurface);

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool toggleFullscreen = false;

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
		if (allClosed && !IsRestarting)
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	break;
	case WM_SIZE:
		Resized = (wparam != SIZE_MINIMIZED);
		break;
	case WM_SYSCHAR:
		toggleFullscreen = (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN));
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
		else if (wparam == VK_F11)
		{
			IsRestarting = true;
			TestShutdown();
			TestInitialize();
		}
	}

	if ((Resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggleFullscreen)
	{
		Platform::Window win{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd,GWLP_USERDATA)} };
		for (uint32 i = 0; i < _countof(Surfaces); i++)
		{
			if (win.ID() == Surfaces[i].Window.ID())
			{
				if (toggleFullscreen)
				{
					win.SetFullScreen(!win.IsFullScreen());
					return 0;
				}
				else
				{
					Surfaces[i].Surface.Resize(win.Width(), win.Height());
					Resized = false;
				}
				break;
			}

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

bool TestInitialize()
{
	while (!CompileShaders())
	{
		if (MessageBox(nullptr, L"エンジンシェーダーをコンパイルできませんでした", L"Shader Compile Error", MB_RETRYCANCEL) != IDRETRY)
			return false;
	}

	if (!Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12))
		return false;

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

	IsRestarting = false;
	return true;
}

bool EngineTest::Initialize()
{
	return TestInitialize();
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

void TestShutdown()
{
	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		DestroyRenderSurface(Surfaces[i]);
	}
	Graphics::Shutdown();
}

void EngineTest::Shutdown()
{
	TestShutdown();
}

#endif // TEST_RENDERER