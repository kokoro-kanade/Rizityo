#include "CommonHeaders.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "D3D12Content.h"
#include "D3D12Camera.h"
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

		graphicsInterface.Resources.AddSubmesh = Content::Submesh::Add;
		graphicsInterface.Resources.RemoveSubmesh = Content::Submesh::Remove;

		graphicsInterface.Camera.Create = Camera::CreateCamera;
		graphicsInterface.Camera.Remove = Camera::RemoveCamera;
		graphicsInterface.Camera.SetParameter = Camera::SetParameter;
		graphicsInterface.Camera.GetParameter = Camera::GetParameter;

		graphicsInterface.Platform = GraphicsPlatform::Direct3D12;
	}
}
