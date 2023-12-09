#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Graphics
{

    DEFINE_ID_TYPE(CameraID);

    class Camera
    {
    public:
        enum class Type : uint32
        {
            Perspective,
            Orthographic
        };

        constexpr explicit Camera(CameraID id) : _ID{ id } {}
        constexpr Camera() = default;
        constexpr CameraID ID() const { return _ID; }
        constexpr bool IsValid() const { return ID::IsValid(_ID); }

        void SetUpVector(Math::DX_Vector3 up) const;

        void SetFieldOfView(float32 fov) const;
        void SetAspectRatio(float32 aspect_ratio) const;

        void SetViewWidth(float32 width) const;
        void SetViewHeight(float32 height) const;

        void SetRange(float32 near_z, float32 far_z) const;

        Math::DX_Matrix4x4 View() const;
        Math::DX_Matrix4x4 Projection() const;
        Math::DX_Matrix4x4 InverseProjection() const;
        Math::DX_Matrix4x4 ViewProjection() const;
        Math::DX_Matrix4x4 InverseViewProjection() const;
        Math::DX_Vector3 UpVector() const;

        float32 NearZ() const;
        float32 FarZ() const;

        float32 FieldOfView() const;
        float32 AspectRatio() const;

        float32 ViewWidth() const;
        float32 ViewHeight() const;

        Type ProjectionType() const;
        ID::IDType EntityID() const;

    private:
        CameraID _ID{ ID::INVALID_ID };
    };

    // TODO: カメラを作成するAPIを公開
    //       内部でエンティティを作成する
    //       

}