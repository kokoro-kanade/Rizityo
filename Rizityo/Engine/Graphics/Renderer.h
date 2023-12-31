#pragma once
#include "CommonHeaders.h"
#include "Core/Platform/Window.h"
#include "API/Camera.h"
#include "API/Light.h"

namespace Rizityo::Graphics
{
	enum class GraphicsPlatform : uint32
	{
		Direct3D12 = 0,
		Vulkan = 1,
		OpenGL = 2,
	};

    bool Initialize(GraphicsPlatform platform);
    void Shutdown();

    struct FrameInfo
    {
        ID::IDType* RenderItemIDs = nullptr;
        float32* Thresholds = nullptr;
        uint32 RenderItemCount = 0;
        CameraID CamerID{ ID::INVALID_ID };
        uint64 LightSetKey = 0;
        float32 LastFrameTime = 16.7f;
        float32 AverageFrameTime = 16.7f;
    };

    // サーフェス
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
		void Render(FrameInfo info) const;

	private:

		SurfaceID _ID{ ID::INVALID_ID };
	};

	struct RenderSurface
	{
		Rizityo::Platform::Window Window{};
        Rizityo::Graphics::Surface Surface{};
	};

    Surface CreateSurface(Platform::Window window);
    void RemoveSurface(SurfaceID id);


    // リソース
    struct PrimitiveTopology {
        enum Type : uint32 {
            PointList = 1,
            LineList,
            LineStrip,
            TriangleList,
            TriangleStrip,
            Count
        };
    };

	ID::IDType AddSubmesh(const uint8*& data);
	void RemoveSubmesh(ID::IDType id);

    struct ShaderFlags {
        enum Flags : uint32 {
            None = 0x0,
            Vertex = 0x01,
            Hull = 0x02,
            Domain = 0x04,
            Geometry = 0x08,
            Pixel = 0x10,
            Compute = 0x20,
            Amplification = 0x40,
            Mesh = 0x80,
        };
    };

    struct ShaderType {
        enum Type : uint32 {
            Vertex = 0,
            Hull,
            Domain,
            Geometry,
            Pixel,
            Compute,
            Amplification,
            Mesh,
            Count
        };
    };

    struct MaterialType {
        enum Type : uint32 {
            Opaque,
            Count
        };
    };

    struct MaterialInitInfo
    {
        MaterialType::Type Type;
        uint32 TextureCount = 0; // テクスチャはない可能性がある(TextureCount == 0)
        ID::IDType ShaderIDs[ShaderType::Count]{ ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID, ID::INVALID_ID };
        ID::IDType* TextureIDs = nullptr;
    };

    ID::IDType AddMaterial(MaterialInitInfo info);
    void RemoveMaterial(ID::IDType id);

    ID::IDType AddRenderItem(ID::IDType entityID, ID::IDType geometryContentID,
                               uint32 materialCount, const ID::IDType* const materialIDs);
    void RemoveRenderItem(ID::IDType id);

	const char* GetEngineShadersPath();
	const char* GetEngineShadersPath(GraphicsPlatform platform);


    // カメラ
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
        Math::DX_Vector3 UpVector;
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
            NearZ = 0.01f;
            FarZ = 1000.f;
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
            NearZ = 0.01f;
            FarZ = 1000.f;
        }
    };

    Camera CreateCamera(CameraInitInfo info);
    void RemoveCamera(CameraID id);


    // ライト
    struct DirectionalLightParams {};

    struct PointLightParams
    {
        Math::DX_Vector3 Attenuation;
        float32 Range;
    };

    struct SpotLightParams
    {
        Math::DX_Vector3 Attenuation;
        float32 Range;
        float32 InnerAngle; // [0, PI)
        float32 OuterAngle; // [Inner, PI)
    };

    struct LightInitInfo
    {
        uint64 LightSetKey = 0;
        ID::IDType EntityID{ ID::INVALID_ID };
        Light::Type Type{};
        float32 Intensity = 1.f;
        Math::DX_Vector3 Color{ 1.f, 1.f, 1.f };
        union
        {
            DirectionalLightParams DirectionalParams;
            PointLightParams PointParams;
            SpotLightParams SpotParams;
        };
        bool IsEnabled = true;
    };

    struct LightParameter {
        enum Parameter : uint32 {
            IsEnabled,
            Intensity,
            Color,
            Type,
            EntityID,
            Count
        };
    };

    Light CreateLight(LightInitInfo info);
    void RemoveLight(LightID id, uint64 lightSetKey);
}