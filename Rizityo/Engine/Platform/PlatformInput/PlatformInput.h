#pragma once
#ifdef _WIN64
#include "CommonHeaders.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Rizityo::Platform
{
	HRESULT ProcessInputMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

#endif // !_WIN64