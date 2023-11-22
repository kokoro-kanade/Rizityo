#include <d3d12shader.h>
#include <dxcapi.h>
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "Direct3D12/D3D12Interface.h"

namespace Rizityo::Graphics
{
	namespace
	{
		constexpr const char* EngineShadersPaths[]
		{
			".\\Shaders\\D3D12\\Shaders.bin",
			/*".\\Shaders\\Vulkan\\Shaders.bin"*/
		};

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

			assert(graphicsInterface.Platform == platform);

			return true;
		}

	} // –³–¼‹óŠÔ

	bool Initialize(GraphicsPlatform platform)
	{
		return SetPlatformInterface(platform) && graphicsInterface.Initialize();
	}

	void Shutdown()
	{
		if (graphicsInterface.Platform != (GraphicsPlatform)-1)
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

	ID::IDType AddSubmesh(const uint8*& data)
	{
		return graphicsInterface.Resources.AddSubmesh(data);
	}

	void RemoveSubmesh(ID::IDType id)
	{
		graphicsInterface.Resources.RemoveSubmesh(id);
	}

	const char* GetEngineShadersPath()
	{
		
		return EngineShadersPaths[(uint32)graphicsInterface.Platform];
	}

	const char* GetEngineShadersPath(GraphicsPlatform platform)
	{
		return EngineShadersPaths[(uint32)platform];
	}

	Camera CreateCamera(CameraInitInfo info)
	{
		return graphicsInterface.Camera.Create(info);
	}

	void RemoveCamera(CameraID id)
	{
		graphicsInterface.Camera.Remove(id);
	}

	void Camera::SetUpVector(Math::Vector3 up) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::UpVector, &up, sizeof(up));
	}

	void Camera::SetFieldOfView(float32 fov) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::FieldOfView, &fov, sizeof(fov));
	}

	void Camera::SetAspectRatio(float32 aspectRatio) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
	}

	void Camera::SetViewWidth(float32 width) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::ViewWidth, &width, sizeof(width));
	}

	void Camera::SetViewHeight(float32 height) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::ViewHeight, &height, sizeof(height));
	}

	void Camera::SetRange(float32 nearZ, float32 farZ) const
	{
		assert(IsValid());
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		graphicsInterface.Camera.SetParameter(_ID, CameraParameter::FarZ, &farZ, sizeof(farZ));
	}

	Math::Matrix4x4 Camera::View() const
	{
		assert(IsValid());
		Math::Matrix4x4 matrix;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::View, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::Matrix4x4 Camera::Projection() const
	{
		assert(IsValid());
		Math::Matrix4x4 matrix;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::Projection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::Matrix4x4 Camera::InverseProjection() const
	{
		assert(IsValid());
		Math::Matrix4x4 matrix;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::InverseProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::Matrix4x4 Camera::ViewProjection() const
	{
		assert(IsValid());
		Math::Matrix4x4 matrix;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::ViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::Matrix4x4 Camera::InverseViewProjection() const
	{
		assert(IsValid());
		Math::Matrix4x4 matrix;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::InverseViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::Vector3 Camera::UpVector() const
	{
		assert(IsValid());
		Math::Vector3 up_vector;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::UpVector, &up_vector, sizeof(up_vector));
		return up_vector;
	}

	float32 Camera::NearZ() const
	{
		assert(IsValid());
		float32 near_z;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::NearZ, &near_z, sizeof(near_z));
		return near_z;
	}

	float32 Camera::FarZ() const
	{
		assert(IsValid());
		float32 far_z;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::FarZ, &far_z, sizeof(far_z));
		return far_z;
	}

	float32 Camera::FieldOfView() const
	{
		assert(IsValid());
		float32 fov;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::FieldOfView, &fov, sizeof(fov));
		return fov;
	}

	float32 Camera::AspectRatio() const
	{
		assert(IsValid());
		float32 aspect_ratio;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::AspectRatio, &aspect_ratio, sizeof(aspect_ratio));
		return aspect_ratio;
	}

	float32 Camera::ViewWidth() const
	{
		assert(IsValid());
		float32 width;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::ViewWidth, &width, sizeof(width));
		return width;
	}

	float32 Camera::ViewHeight() const
	{
		assert(IsValid());
		float32 height;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::ViewHeight, &height, sizeof(height));
		return height;
	}

	Camera::Type Camera::ProjectionType() const
	{
		assert(IsValid());
		Camera::Type type;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::Type, &type, sizeof(type));
		return type;
	}

	ID::IDType Camera::EntityID() const
	{
		assert(IsValid());
		ID::IDType entity_id;
		graphicsInterface.Camera.GetParameter(_ID, CameraParameter::EntityID, &entity_id, sizeof(entity_id));
		return entity_id;
	}
}