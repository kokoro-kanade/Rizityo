#pragma once
#include "CommonHeaders.h"
#include "Platform/Window.h"
#include "EngineAPI/Camera.h"

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
	void Shutdown();

	// TODO?: ï ÇÃÉwÉbÉ_Å[Ç…èëÇ≠Ç©Ç«Ç§Ç©
	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);

	ID::IDType AddSubmesh(const uint8*& data);
	void RemoveSubmesh(ID::IDType id);

	const char* GetEngineShadersPath();
	const char* GetEngineShadersPath(GraphicsPlatform platform);

    struct CameraParameter
    {
        enum Parameter : uint32 
        {
            UpVector,
            FieldOfView,
            AspectRatio,
            ViewWidth,
            ViewHeight,
            NearZ,
            FarZ,
            View,
            Projection,
            InverseProjection,
            ViewProjection,
            InverseViewProjection,
            Type,
            EntityID,
            Count
        };
    };

    struct CameraInitInfo
    {
        ID::IDType EntityID{ ID::INVALID_ID };
        Camera::Type Type{};
        Math::Vector3 UpVector;
        union 
        {
            float32 FieldOfView;
            float32 ViewWidth;
        };
        union 
        {
            float32 AspectRatio;
            float32 ViewHeight;
        };
        float32 NearZ;
        float32 FarZ;
    };

    struct PerspectiveCameraInitInfo : public CameraInitInfo
    {
        explicit PerspectiveCameraInitInfo(ID::IDType entityID)
        {
            assert(ID::IsValid(entityID));
            EntityID = entityID;
            Type = Camera::Type::Perspective;
            UpVector = { 0.f, 1.f, 0.f };
            FieldOfView = 0.25f;
            AspectRatio = 16.f / 10.f;
            NearZ = 0.001f;
            FarZ = 10000.f;
        }
    };

    struct OrthographicCameraInitInfo : public CameraInitInfo
    {
        explicit OrthographicCameraInitInfo(ID::IDType entityID)
        {
            assert(ID::IsValid(entityID));
            EntityID = entityID;
            Type = Camera::Type::Orthographic;
            UpVector = { 0.f, 1.f, 0.f };
            ViewWidth = 1920;
            ViewHeight = 1080;
            NearZ = 0.001f;
            FarZ = 10000.f;
        }
    };

    Camera CreateCamera(CameraInitInfo info);
    void RemoveCamera(CameraID id);
}