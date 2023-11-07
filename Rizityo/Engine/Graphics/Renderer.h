#pragma once
#include "CommonHeaders.h"
#include "../Platform/Window.h"

namespace Rizityo::Graphics
{
	class Surface
	{};

	struct RenderSurface
	{
		Platform::Window Window{};
		Surface Surface{};
	};
}