#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Camera
{

    class D3D12Camera
    {
    public:

        explicit D3D12Camera(CameraInitInfo info);

        void Update();

        void SetUpVector(Math::DX_Vector3 up);

        constexpr void SetFieldOfView(float32 fov);
        constexpr void SetAspectRatio(float32 aspect_ratio);

        constexpr void SetViewWidth(float32 width);
        constexpr void SetViewHeight(float32 height);

        constexpr void SetNearZ(float32 near_z);
        constexpr void SetFarZ(float32 far_z);

        [[nodiscard]] constexpr DirectX::XMMATRIX View() const { return _View; }
        [[nodiscard]] constexpr DirectX::XMMATRIX Projection() const { return _Projection; }
        [[nodiscard]] constexpr DirectX::XMMATRIX InverseProjection() const { return _InverseProjection; }
        [[nodiscard]] constexpr DirectX::XMMATRIX ViewProjection() const { return _ViewProjection; }
        [[nodiscard]] constexpr DirectX::XMMATRIX InverseViewProjection() const { return _InverseViewProjection; }
        [[nodiscard]] constexpr DirectX::XMVECTOR Position() const { return _Position; }
        [[nodiscard]] constexpr DirectX::XMVECTOR Direction() const { return _Direction; }
        [[nodiscard]] constexpr DirectX::XMVECTOR UpVector() const { return _UpVector; }

        [[nodiscard]] constexpr float32 NearZ() const { return _NearZ; }
        [[nodiscard]] constexpr float32 FarZ() const { return _FarZ; }

        [[nodiscard]] constexpr float32 FieldOfView() const { return _FieldOfView; }
        [[nodiscard]] constexpr float32 AspectRatio() const { return _AspectRatio; }

        [[nodiscard]] constexpr float32 ViewWidth() const { return _ViewWidth; }
        [[nodiscard]] constexpr float32 ViewHeight() const { return _ViewHeight; }

        [[nodiscard]] constexpr Graphics::Camera::Type RrojectionType() const { return _ProjectionType; }
        [[nodiscard]] constexpr ID::IDType EntityID() const { return _EntityID; }

    private:

        DirectX::XMMATRIX _View;
        DirectX::XMMATRIX _Projection;
        DirectX::XMMATRIX _InverseProjection;
        DirectX::XMMATRIX _ViewProjection;
        DirectX::XMMATRIX _InverseViewProjection;
        DirectX::XMVECTOR _Position{};
        DirectX::XMVECTOR _Direction{};
        DirectX::XMVECTOR _UpVector;

        float32 _NearZ;
        float32 _FarZ;

        union // ìßéãìäâe
        {
            float32 _FieldOfView;
            float32 _ViewWidth;
        };
        union // ïΩçsìäâe
        {
            float32 _AspectRatio;
            float32 _ViewHeight;
        };

        Graphics::Camera::Type _ProjectionType;
        ID::IDType _EntityID;

        bool _UpdateFlag;
    };

    Graphics::Camera CreateCamera(CameraInitInfo info);
    void RemoveCamera(CameraID id);
    void SetParameter(CameraID id, CameraParameter::Parameter parameter, const void* const data, uint32 data_size);
    void GetParameter(CameraID id, CameraParameter::Parameter parameter, OUT void* const data, uint32 data_size);
    [[nodiscard]] D3D12Camera& GetCamera(CameraID id);
}