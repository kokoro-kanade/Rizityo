#include "Platform.h"
#include "PlatformTypes.h"

namespace Rizityo::Platform
{
#ifdef _WIN64

	namespace
	{
		// TODO? PlatfromTypes.hに移動
		struct WindowInfo
		{
			HWND Hwnd = nullptr;
			RECT ClientArea{ 0, 0, 1920, 1080 };
			RECT FullScreenArea{};
			POINT TopLeft{ 0, 0 };
			DWORD Style{ WS_VISIBLE };
			bool IsFullScreen = false;
			bool IsClosed = false;
		};

		Utility::Vector<int> v;

		Utility::FreeList<WindowInfo> Windows;

		bool Resized = false;
	} // 変数

	namespace
	{
		WindowInfo& GetWindowInfoFromId(WindowID id)
		{
			assert(Windows[id].Hwnd);
			return Windows[id];
		}

		WindowInfo& GetWindowInfoFromHandle(WindowHandle handle)
		{
			const WindowID id{ (ID::IDType)GetWindowLongPtr(handle, GWLP_USERDATA) };
			return GetWindowInfoFromId(id);
		}

		LRESULT CALLBACK InternalWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg)
			{
			case WM_NCCREATE:
			{
				// windowIDをユーザデータ領域に格納
				DEBUG_ONLY(SetLastError(0));
				const WindowID id{ Windows.Add() };
				Windows[id].Hwnd = hwnd;
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)id);
				assert(GetLastError() == 0);
			}
			case WM_DESTROY:
				GetWindowInfoFromHandle(hwnd).IsClosed = true;
				break;
			case WM_SIZE:
				Resized = (wparam != SIZE_MINIMIZED);
				break;
			default:
				break;
			}

			if (Resized && GetAsyncKeyState(VK_LBUTTON) >= 0)
			{
				WindowInfo& info{ GetWindowInfoFromHandle(hwnd) };
				assert(info.Hwnd);
				GetClientRect(info.Hwnd, info.IsFullScreen ? &info.FullScreenArea : &info.ClientArea);
				Resized = false;
			}

			LONG_PTR longPtr{ GetWindowLongPtr(hwnd, 0) };
			return longPtr ? ((WindowProc)longPtr)(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
		}

		void ResizeWindow(const WindowInfo& info, const RECT& area)
		{
			RECT windowRect{ area };
			AdjustWindowRect(&windowRect, info.Style, FALSE);
			const int32 width = windowRect.right - windowRect.left;
			const int32 height = windowRect.bottom - windowRect.top;
			MoveWindow(info.Hwnd, info.TopLeft.x, info.TopLeft.y, width, height, true);
		}

		void ResizeWindow(WindowID id, uint32 width, uint32 height)
		{
			WindowInfo& info{ GetWindowInfoFromId(id) };

			// 親のウィンドウがいるなら内部の情報だけ更新
			if (info.Style & WS_CHILD)
			{
				GetClientRect(info.Hwnd, &info.ClientArea);
			}
			else
			{
				// スクリーンの解像度を変えたときにも対応できるようにフルスクリーンでもリサイズする
				RECT& area{ info.IsFullScreen ? info.FullScreenArea : info.ClientArea };
				area.bottom = area.top + height;
				area.right = area.left + width;

				ResizeWindow(info, area);
			}

		}

		void SetWindowFullScreen(WindowID id, bool isFullScreen)
		{
			WindowInfo& info{ GetWindowInfoFromId(id) };
			if (info.IsFullScreen == isFullScreen)
				return;

			info.IsFullScreen = isFullScreen;

			if (isFullScreen)
			{
				// フルスクリーンから戻すときのために現在のウィンドウの大きさを保存しておく
				GetClientRect(info.Hwnd, &info.ClientArea);
				RECT rect;
				GetWindowRect(info.Hwnd, &rect);
				info.TopLeft.x = rect.left;
				info.TopLeft.y = rect.top;
				SetWindowLongPtr(info.Hwnd, GWL_STYLE, 0);
				ShowWindow(info.Hwnd, SW_MAXIMIZE);
			}
			else
			{
				SetWindowLongPtr(info.Hwnd, GWL_STYLE, info.Style);
				ResizeWindow(info, info.ClientArea);
				ShowWindow(info.Hwnd, SW_SHOWNORMAL);
			}
			
		}

		bool IsWindowFullScreen(WindowID id)
		{
			return GetWindowInfoFromId(id).IsFullScreen;
		}

		WindowHandle GetWindowHandle(WindowID id)
		{
			return GetWindowInfoFromId(id).Hwnd;
		}

		void SetWindowCaption(WindowID id, const wchar_t* caption)
		{
			WindowInfo& info{ GetWindowInfoFromId(id) };
			SetWindowText(info.Hwnd, caption);
		}

		Math::U32Vector4 GetWindowSize(WindowID id)
		{
			WindowInfo& info{ GetWindowInfoFromId(id) };
			RECT& area{ info.IsFullScreen ? info.FullScreenArea : info.ClientArea };
			return { (uint32)area.left, (uint32)area.top, (uint32)area.right, (uint32)area.bottom };
		}

		bool IsWindowClosed(WindowID id)
		{
			return GetWindowInfoFromId(id).IsClosed;
		}

	} // 関数

	Window CreateMyWindow(const WindowInitInfo* const initInfo/* = nullptr */)
	{
		WindowProc callback{ initInfo ? initInfo->Callback : nullptr };
		WindowHandle parent{ initInfo ? initInfo->Parent : nullptr };

		// Windowクラスの設定
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = InternalWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = callback ? sizeof(callback) : 0; // callbackを入れておくための領域サイズ
		wc.hInstance = 0;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(20, 48, 76));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"RizityoWindow";
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		// ウィンドウクラスの登録
		RegisterClassEx(&wc);

		WindowInfo info{};
		info.ClientArea.right = (initInfo && initInfo->Width) ? info.ClientArea.left + initInfo->Width : info.ClientArea.right;
		info.ClientArea.bottom = (initInfo && initInfo->Height) ? info.ClientArea.top + initInfo->Height : info.ClientArea.bottom;
		info.Style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;
		RECT rect{ info.ClientArea };

		AdjustWindowRect(&rect, info.Style, FALSE);

		const wchar_t* caption = (initInfo && initInfo->Caption) ? initInfo->Caption : L"Rizityo Game";
		const int32 left = initInfo ? initInfo->Left : info.TopLeft.x;
		const int32 top = initInfo ? initInfo->Top : info.TopLeft.y;
		const int32 width = rect.right - rect.left;
		const int32 height = rect.bottom - rect.top;

		// ウィンドウクラスのインスタンスを作成
		info.Hwnd = CreateWindowEx(
			0,						 // extended style
			wc.lpszClassName,		 // window class name
			caption,				 // instance title
			info.Style,				 // window style
			left,					 // initial window position
			top,					 // 
			width,					 // initial window dimensions
			height,					 //
			parent,					 // handle to parent window
			NULL,					 // handle to menu
			NULL,					 // instance of this application
			NULL					 // extra creation parameters
		);

		if (info.Hwnd)
		{
			// windowメッセージを処理するコールバック関数の設定
			DEBUG_ONLY(SetLastError(0));
			if (callback)
				SetWindowLongPtr(info.Hwnd, 0, (LONG_PTR)callback);
			assert(GetLastError() == 0);

			ShowWindow(info.Hwnd, SW_SHOWNORMAL);
			UpdateWindow(info.Hwnd);

			WindowID id{ (ID::IDType)GetWindowLongPtr(info.Hwnd, GWLP_USERDATA) };
			Windows[id] = info;
			return Window{ id };
		}

		return {};
		
	}

	void RemoveMyWindow(WindowID id)
	{
		WindowInfo& info{ GetWindowInfoFromId(id) };
		DestroyWindow(info.Hwnd);
		Windows.Remove(id);
	}

#elif LINUX

#else
#error "少なくとも一つのプラットフォームで実装してください"
#endif // _WIN64

	// プラットフォーム非依存
	void Window::SetFullScreen(bool isFullScreen) const
{
	assert(IsValid());
	SetWindowFullScreen(_ID, isFullScreen);
}
	
	bool Window::IsFullScreen() const
{
	assert(IsValid());
	return IsWindowFullScreen(_ID);
}
	
	void* Window::Handle() const
{
	assert(IsValid());
	return GetWindowHandle(_ID);
}
	
	void Window::SetCaption(const wchar_t* caption) const
{
	assert(IsValid());
	SetWindowCaption(_ID, caption);
}
	
	Math::U32Vector4 Window::Size() const
	{
		assert(IsValid());
		return GetWindowSize(_ID);
	}

	void Window::Resize(uint32 width, uint32 height) const
	{
		assert(IsValid());
		ResizeWindow(_ID, width, height);
	}
	
	uint32 Window::Width() const
	{
		Math::U32Vector4 size{ Size() };
		return size.z - size.x;
	}

	uint32 Window::Height() const
	{
		Math::U32Vector4 size{ Size() };
		return size.w - size.y;
	}
	
	bool Window::IsClosed() const
	{
		assert(IsValid());
		return IsWindowClosed(_ID);
	}

} // Platform