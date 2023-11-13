#include "Renderer.h"
#include "GraphicsInterface.h"
#include "Direct3D12/D3D12Interface.h"

namespace Rizityo::Graphics
{
	namespace
	{
		GraphicsInterface graphicsInterface{};

		bool SetPlatformInterface(GraphicsPlatform platform)
		{
			switch (platform)
			{
			case Rizityo::Graphics::GraphicsPlatform::Direct3D12:
				D3D12::SetInterface(graphicsInterface);
				break;
			case Rizityo::Graphics::GraphicsPlatform::Vulkan:
				break;
			case Rizityo::Graphics::GraphicsPlatform::OpenGL:
				break;
			default:
				return false;
			}

			return true;
		}

	} // –³–¼‹óŠÔ

	bool Initialize(GraphicsPlatform platform)
	{
		return SetPlatformInterface(platform) && graphicsInterface.Initialize();
	}

	void Shutdown()
	{
		graphicsInterface.Shutdown();
	}

	Surface CreateSurface(Platform::Window window)
	{
		return graphicsInterface.Surface.Create(window);
	}

	void RemoveSurface(SurfaceID id)
	{
		assert(ID::IsValid(id));
		graphicsInterface.Surface.Remove(id);
	}

	void Surface::Resize(uint32 width, uint32 height) const
	{
		assert(IsValid());
		graphicsInterface.Surface.Resize(_ID, width, height);
	}

	uint32 Surface::Width() const
	{
		assert(IsValid());
		return graphicsInterface.Surface.Width(_ID);
	}

	uint32 Surface::Height() const
	{
		assert(IsValid());
		return graphicsInterface.Surface.Height(_ID);
	}

	void Surface::Render() const
	{
		assert(IsValid());
		graphicsInterface.Surface.Render(_ID);
	}
}