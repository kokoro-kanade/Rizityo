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
			void(*Render)(SurfaceID, FrameInfo);
		} Surface;

		struct
		{
			ID::IDType(*AddSubmesh)(const uint8*&);
			void (*RemoveSubmesh)(ID::IDType);
			ID::IDType(*AddMaterial)(MaterialInitInfo);
			void (*RemoveMaterial)(ID::IDType);
			ID::IDType(*AddRenderItem)(ID::IDType, ID::IDType, uint32, const ID::IDType* const);
			void (*RemoveRenderItem)(ID::IDType);
		} Resources;

		struct
		{
			Camera(*Create)(CameraInitInfo);
			void(*Remove)(CameraID);
			void(*SetParameter)(CameraID, CameraParameter::Parameter, const void* const, uint32);
			void(*GetParameter)(CameraID, CameraParameter::Parameter, OUT void* const, uint32);
		} Camera;

		struct {
			Light(*Create)(LightInitInfo);
			void(*Remove)(LightID, uint64);
			void(*SetParameter)(LightID, uint64, LightParameter::Parameter, const void* const, uint32);
			void(*GetParameter)(LightID, uint64, LightParameter::Parameter, void* const, uint32);
		} Light;

		GraphicsPlatform Platform = (GraphicsPlatform)-1;
	};
}