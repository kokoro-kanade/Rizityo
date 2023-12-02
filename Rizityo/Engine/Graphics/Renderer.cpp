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

		GraphicsInterface GFXInterface{};

		bool SetPlatformInterface(GraphicsPlatform platform, OUT GraphicsInterface& gi)
		{
			switch (platform)
			{
			case Rizityo::Graphics::GraphicsPlatform::Direct3D12:
				D3D12::SetInterface(gi);
				break;
			case Rizityo::Graphics::GraphicsPlatform::Vulkan:
				break;
			case Rizityo::Graphics::GraphicsPlatform::OpenGL:
				break;
			default:
				return false;
			}

			assert(gi.Platform == platform);

			return true;
		}

	} // 無名空間

	bool Initialize(GraphicsPlatform platform)
	{
		return SetPlatformInterface(platform, GFXInterface) && GFXInterface.Initialize();
	}

	void Shutdown()
	{
		if (GFXInterface.Platform != (GraphicsPlatform)-1)
			GFXInterface.Shutdown();
	}


	// サーフェス
	Surface CreateSurface(Platform::Window window)
	{
		return GFXInterface.Surface.Create(window);
	}

	void RemoveSurface(SurfaceID id)
	{
		assert(ID::IsValid(id));
		GFXInterface.Surface.Remove(id);
	}

	void Surface::Resize(uint32 width, uint32 height) const
	{
		assert(IsValid());
		GFXInterface.Surface.Resize(_ID, width, height);
	}

	uint32 Surface::Width() const
	{
		assert(IsValid());
		return GFXInterface.Surface.Width(_ID);
	}

	uint32 Surface::Height() const
	{
		assert(IsValid());
		return GFXInterface.Surface.Height(_ID);
	}

	void Surface::Render(FrameInfo info) const
	{
		assert(IsValid());
		GFXInterface.Surface.Render(_ID, info);
	}


	// リソース
	ID::IDType AddSubmesh(const uint8*& data)
	{
		return GFXInterface.Resources.AddSubmesh(data);
	}

	void RemoveSubmesh(ID::IDType id)
	{
		GFXInterface.Resources.RemoveSubmesh(id);
	}

	ID::IDType AddMaterial(MaterialInitInfo info)
	{
		return GFXInterface.Resources.AddMaterial(info);
	}

	void RemoveMaterial(ID::IDType id)
	{
		GFXInterface.Resources.RemoveMaterial(id);
	}

	ID::IDType AddRenderItem(ID::IDType entityID, ID::IDType geometryContentID,
							 uint32 materialCount, const ID::IDType* const materialIDs)
	{
		return GFXInterface.Resources.AddRenderItem(entityID, geometryContentID, materialCount, materialIDs);
	}

	void RemoveRenderItem(ID::IDType id)
	{
		GFXInterface.Resources.RemoveRenderItem(id);
	}

	const char* GetEngineShadersPath()
	{
		
		return EngineShadersPaths[(uint32)GFXInterface.Platform];
	}

	const char* GetEngineShadersPath(GraphicsPlatform platform)
	{
		return EngineShadersPaths[(uint32)platform];
	}


	// カメラ
	Camera CreateCamera(CameraInitInfo info)
	{
		return GFXInterface.Camera.Create(info);
	}

	void RemoveCamera(CameraID id)
	{
		GFXInterface.Camera.Remove(id);
	}

	void Camera::SetUpVector(Math::DX_Vector3 up) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::UpVector, &up, sizeof(up));
	}

	void Camera::SetFieldOfView(float32 fov) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::FieldOfView, &fov, sizeof(fov));
	}

	void Camera::SetAspectRatio(float32 aspectRatio) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
	}

	void Camera::SetViewWidth(float32 width) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::ViewWidth, &width, sizeof(width));
	}

	void Camera::SetViewHeight(float32 height) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::ViewHeight, &height, sizeof(height));
	}

	void Camera::SetRange(float32 nearZ, float32 farZ) const
	{
		assert(IsValid());
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		GFXInterface.Camera.SetParameter(_ID, CameraParameter::FarZ, &farZ, sizeof(farZ));
	}

	Math::DX_Matrix4x4 Camera::View() const
	{
		assert(IsValid());
		Math::DX_Matrix4x4 matrix;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::View, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::DX_Matrix4x4 Camera::Projection() const
	{
		assert(IsValid());
		Math::DX_Matrix4x4 matrix;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::Projection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::DX_Matrix4x4 Camera::InverseProjection() const
	{
		assert(IsValid());
		Math::DX_Matrix4x4 matrix;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::InverseProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::DX_Matrix4x4 Camera::ViewProjection() const
	{
		assert(IsValid());
		Math::DX_Matrix4x4 matrix;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::ViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::DX_Matrix4x4 Camera::InverseViewProjection() const
	{
		assert(IsValid());
		Math::DX_Matrix4x4 matrix;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::InverseViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	Math::DX_Vector3 Camera::UpVector() const
	{
		assert(IsValid());
		Math::DX_Vector3 up_vector;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::UpVector, &up_vector, sizeof(up_vector));
		return up_vector;
	}

	float32 Camera::NearZ() const
	{
		assert(IsValid());
		float32 near_z;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::NearZ, &near_z, sizeof(near_z));
		return near_z;
	}

	float32 Camera::FarZ() const
	{
		assert(IsValid());
		float32 far_z;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::FarZ, &far_z, sizeof(far_z));
		return far_z;
	}

	float32 Camera::FieldOfView() const
	{
		assert(IsValid());
		float32 fov;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::FieldOfView, &fov, sizeof(fov));
		return fov;
	}

	float32 Camera::AspectRatio() const
	{
		assert(IsValid());
		float32 aspect_ratio;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::AspectRatio, &aspect_ratio, sizeof(aspect_ratio));
		return aspect_ratio;
	}

	float32 Camera::ViewWidth() const
	{
		assert(IsValid());
		float32 width;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::ViewWidth, &width, sizeof(width));
		return width;
	}

	float32 Camera::ViewHeight() const
	{
		assert(IsValid());
		float32 height;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::ViewHeight, &height, sizeof(height));
		return height;
	}

	Camera::Type Camera::ProjectionType() const
	{
		assert(IsValid());
		Camera::Type type;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::Type, &type, sizeof(type));
		return type;
	}

	ID::IDType Camera::EntityID() const
	{
		assert(IsValid());
		ID::IDType entityID;
		GFXInterface.Camera.GetParameter(_ID, CameraParameter::EntityID, &entityID, sizeof(entityID));
		return entityID;
	}


	// ライト
	Light CreateLight(LightInitInfo info)
	{
		return GFXInterface.Light.Create(info);
	}

	void RemoveLight(LightID id, uint64 light_set_key)
	{
		GFXInterface.Light.Remove(id, light_set_key);
	}

	void Light::SetEnabled(bool is_enabled) const
	{
		assert(IsValid());
		GFXInterface.Light.SetParameter(_ID, _LightSetKey, LightParameter::IsEnabled, &is_enabled, sizeof(is_enabled));
	}

	void Light::SetIntensity(float32 intensity) const
	{
		assert(IsValid());
		GFXInterface.Light.SetParameter(_ID, _LightSetKey, LightParameter::Intensity, &intensity, sizeof(intensity));
	}

	void Light::SetColor(Math::DX_Vector3 color) const
	{
		assert(IsValid());
		GFXInterface.Light.SetParameter(_ID, _LightSetKey, LightParameter::Color, &color, sizeof(color));
	}


	bool Light::IsEnabled() const
	{
		assert(IsValid());
		bool is_enabled;
		GFXInterface.Light.GetParameter(_ID, _LightSetKey, LightParameter::IsEnabled, &is_enabled, sizeof(is_enabled));
		return is_enabled;
	}

	float32 Light::GetIntensity() const
	{
		assert(IsValid());
		float32 intensity;
		GFXInterface.Light.GetParameter(_ID, _LightSetKey, LightParameter::Intensity, &intensity, sizeof(intensity));
		return intensity;
	}

	Math::DX_Vector3 Light::GetColor() const
	{
		assert(IsValid());
		Math::DX_Vector3 color;
		GFXInterface.Light.GetParameter(_ID, _LightSetKey, LightParameter::Color, &color, sizeof(color));
		return color;
	}

	Light::Type Light::GetLightType() const
	{
		assert(IsValid());
		Type type;
		GFXInterface.Light.GetParameter(_ID, _LightSetKey, LightParameter::Type, &type, sizeof(type));
		return type;
	}

	ID::IDType Light::GetEntityID() const
	{
		assert(IsValid());
		ID::IDType id;
		GFXInterface.Light.GetParameter(_ID, _LightSetKey, LightParameter::EntityID, &id, sizeof(id));
		return id;
	}

}