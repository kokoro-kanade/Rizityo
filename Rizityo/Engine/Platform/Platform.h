#pragma once
#include "CommonHeaders.h"
#include "Window.h"

namespace Rizityo::Platform
{
	struct WindowInitInfo;

	// TODO? : window.h�ɂ����H Window���g���ۂɂ����̊֐���m���Ă����K�v���Ȃ��Ȃ炱���ŗǂ�
	Window CreateMyWindow(const WindowInitInfo* const initInfo = nullptr);
	void RemoveMyWindow(WindowID id);
}