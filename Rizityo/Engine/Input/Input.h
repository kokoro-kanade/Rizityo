#pragma once
#include "CommonHeaders.h"
#include "API/Input.h"

namespace Rizityo::Input
{

	void Bind(InputSource source);
	void Unbind(InputSource::Type type, InputCode::Code code);
	void Unbind(uint64 binding);
	void SetInputValue(InputSource::Type type, InputCode::Code code, Math::DX_Vector3 value);
}