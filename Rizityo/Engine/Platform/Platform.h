#pragma once
#include "CommonHeaders.h"
#include "Window.h"

namespace Rizityo::Platform
{
	struct WindowInitInfo;

	// TODO? : window.hにうつす？ Windowを使う際にこれらの関数を知っておく必要がないならここで良い
	Window Create_Window(const WindowInitInfo* const initInfo = nullptr);
	void Remove_Window(WindowID id);
}