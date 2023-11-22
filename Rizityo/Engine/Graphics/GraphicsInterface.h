#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace Rizityo::Graphics
{
	struct GraphicsInterface
	{
		bool(*Initialize)(void);
		void(*Shutdown)(void);

		struct
		{
			Surface(*Create)(Platform::Window);
			void(*Remove)(SurfaceID);
			void(*Resize)(SurfaceID, uint32, uint32);
			uint32(*Width)(SurfaceID);
			uint32(*Height)(SurfaceID);
			void(*Render)(SurfaceID);
		} Surface;

		struct
		{
			ID::IDType(*AddSubmesh)(const uint8*&);
			void (*RemoveSubmesh)(ID::IDType);
		} Resources;

		struct
		{
			Camera(*Create)(CameraInitInfo);
			void(*Remove)(CameraID);
			void(*SetParameter)(CameraID, CameraParameter::Parameter, const void* const, uint32);
			void(*GetParameter)(CameraID, CameraParameter::Parameter, OUT void* const, uint32);
		} Camera;

		GraphicsPlatform Platform = (GraphicsPlatform)-1;
	};
}