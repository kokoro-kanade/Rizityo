#pragma once
#include "CommonHeaders.h"
#include "../Platform/Window.h"

namespace Rizityo::Graphics
{
	DEFINE_ID_TYPE(SurfaceID);

	class Surface
	{
	public:
		constexpr explicit Surface(SurfaceID id) : _ID{ id } {}
		constexpr Surface() : _ID{ ID::INVALID_ID } {}
		constexpr SurfaceID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

		void Resize(uint32 width, uint32 height) const;
		uint32 Width() const;
		uint32 Height() const;
		void Render() const;

	private:

		SurfaceID _ID{ ID::INVALID_ID };
	};

	struct RenderSurface
	{
		Rizityo::Platform::Window Window{};
		Surface Surface{};
	};

	enum class GraphicsPlatform : uint32
	{
		Direct3D12 = 0,
		Vulkan = 1,
		OpenGL = 2,
	};

	bool Initialize(GraphicsPlatform platform);
	void Render();
	void Shutdown();

	// TODO?: ï ÇÃÉwÉbÉ_Å[Ç…èëÇ≠Ç©Ç«Ç§Ç©
	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);

	const char* GetEngineShadersPath();
	const char* GetEngineShadersPath(GraphicsPlatform platform);
}