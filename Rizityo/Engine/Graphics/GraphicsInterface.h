#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"
#include "Platform/Window.h"

namespace Rizityo::Graphics
{
	struct GraphicsInterface
	{
		bool(*Initialize)(void);
		void(*Shutdown)(void);

		struct {
			Surface(*Create)(Platform::Window);
			void(*Remove)(SurfaceID);
			void(*Resize)(SurfaceID, uint32, uint32);
			uint32(*Width)(SurfaceID);
			uint32(*Height)(SurfaceID);
			void(*Render)(SurfaceID);
		} Surface;
	};
}