#pragma once
#include "CommonHeaders.h"
#include "../Window.h"

#ifdef _WIN64

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Rizityo::Platform
{
	using WindowProc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
	using WindowHandle = HWND;

	struct WindowInitInfo
	{
		WindowProc Callback{ nullptr };
		WindowHandle Parent{ nullptr }; // レベルエディタで必要
		const wchar_t* Caption = nullptr;
		int32 Left = 0;
		int32 Top = 0;
		int32 Width = 1920;
		int32 Height = 1080;
	};
}

#endif // _WIN64

namespace Rizityo::Platform
{
	// TODO? : window.hにうつす？ Windowを使う際にこれらの関数を知っておく必要がないならここで良い
	Window CreateMyWindow(const WindowInitInfo* const initInfo = nullptr);
	void RemoveMyWindow(WindowID id);
}