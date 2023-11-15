#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "CommonHeaders.h"
#include "Graphics/GraphicsInterface.h"

namespace Rizityo::Graphics::D3D12
{
	void SetInterface(GraphicsInterface& graphicsInterface)
	{
		graphicsInterface.Initialize = Core::Initialize;
		graphicsInterface.Shutdown = Core::Shutdown;

		graphicsInterface.Surface.Create = Core::CreateSurface;
		graphicsInterface.Surface.Remove = Core::RemoveSurface;
		graphicsInterface.Surface.Resize = Core::ResizeSurface;
		graphicsInterface.Surface.Width = Core::GetSurfaceWidth;
		graphicsInterface.Surface.Height = Core::GetSurfaceHeight;
		graphicsInterface.Surface.Render = Core::RenderSurface;

		graphicsInterface.Platform = GraphicsPlatform::Direct3D12;
	}
}
