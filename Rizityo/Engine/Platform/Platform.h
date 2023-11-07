#pragma once
#include "CommonHeaders.h"
#include "Window.h"

namespace Rizityo::Platform
{
	struct WindowInitInfo;

	Window Create_Window(const WindowInitInfo* const initInfo = nullptr);
	void Remove_Window(WindowId id);
}