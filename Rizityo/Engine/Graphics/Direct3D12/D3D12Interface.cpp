#include "CommonHeaders.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "D3D12Content.h"
#include "D3D12Camera.h"
#include "D3D12Light.h"
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
		graphicsInterface.Resources.AddMaterial = Content::Material::Add;
		graphicsInterface.Resources.RemoveMaterial = Content::Material::Remove;
		graphicsInterface.Resources.AddRenderItem = Content::RenderItem::Add;
		graphicsInterface.Resources.RemoveRenderItem = Content::RenderItem::Remove;

		graphicsInterface.Camera.Create = Camera::CreateCamera;
		graphicsInterface.Camera.Remove = Camera::RemoveCamera;
		graphicsInterface.Camera.SetParameter = Camera::SetParameter;
		graphicsInterface.Camera.GetParameter = Camera::GetParameter;

		graphicsInterface.Light.Create = Light::Create;
		graphicsInterface.Light.Remove = Light::Remove;
		graphicsInterface.Light.SetParameter = Light::SetParameter;
		graphicsInterface.Light.GetParameter = Light::GetParameter;

		graphicsInterface.Platform = GraphicsPlatform::Direct3D12;
	}
}
