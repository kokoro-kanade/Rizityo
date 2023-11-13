#if !defined(SHIPPING)

#include "../Content/ContentLoader.h"
#include "../Components/Script.h"
#include "../Platform/PlatformTypes.h"
#include "../Platform/Platform.h"
#include "../Graphics/Renderer.h"
#include <thread>

using namespace Rizityo;
namespace
{
	Graphics::RenderSurface GameWindow{};

	LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			if (GameWindow.Window.IsClosed())
			{
				PostQuitMessage(0);
				return 0;
			}
		}
		break;
		case WM_SYSCHAR:
			if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
			{
				GameWindow.Window.SetFullScreen(!GameWindow.Window.IsFullScreen());
				return 0;
			}
			break;
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

bool EngineInitialize()
{
	if (!Rizityo::Content::LoadGame())
		return false;
	
	Platform::WindowInitInfo info{ &WinProc, nullptr, L"Rizityo Game" };
	GameWindow.Window = Platform::Create_Window(&info);
	if (!GameWindow.Window.IsValid())
		return false;

	return true;
}

void EngineUpdate()
{
	Rizityo::Script::Update(10.f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
	Platform::Remove_Window(GameWindow.Window.GetID());
	Rizityo::Content::UnLoadGame();
}

#endif