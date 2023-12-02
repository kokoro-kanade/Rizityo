#include "D3D12Camera.h"
#include "EngineAPI/GameEntity.h"

namespace Rizityo::Graphics::D3D12::Camera
{
    namespace
    {

        Utility::FreeList<D3D12Camera> Cameras;

        void SetUpVector(D3D12Camera& camera, const void* const data, uint32 size)
        {
            Math::DX_Vector3 upVector{ *(Math::DX_Vector3*)data };
            assert(sizeof(upVector) == size);
            camera.SetUpVector(upVector);
        }

        constexpr void SetFieldOfView(D3D12Camera& camera, const void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Perspective);
            float32 fov = *(float32*)data;
            assert(sizeof(fov) == size);
            camera.SetFieldOfView(fov);
        }

        constexpr void SetAspectRatio(D3D12Camera& camera, const void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Perspective);
            float32 aspect_ratio = *(float32*)data;
            assert(sizeof(aspect_ratio) == size);
            camera.SetAspectRatio(aspect_ratio);
        }

        constexpr void SetViewWidth(D3D12Camera& camera, const void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Orthographic);
            float32 view_width = *(float32*)data;
            assert(sizeof(view_width) == size);
            camera.SetViewWidth(view_width);
        }

        constexpr void SetViewHeight(D3D12Camera& camera, const void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Orthographic);
            float32 view_height = *(float32*)data;
            assert(sizeof(view_height) == size);
            camera.SetViewHeight(view_height);
        }

        constexpr void SetNearZ(D3D12Camera& camera, const void* const data, uint32 size)
        {
            float32 near_z = *(float32*)data;
            assert(sizeof(near_z) == size);
            camera.SetNearZ(near_z);
        }

        constexpr void SetFarZ(D3D12Camera& camera, const void* const data, uint32 size)
        {
            float32 far_z = *(float32*)data;
            assert(sizeof(far_z) == size);
            camera.SetFarZ(far_z);
        }

        void GetView(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Matrix4x4* const matrix{ (Math::DX_Matrix4x4* const)data };
            assert(sizeof(Math::DX_Matrix4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.View());
        }

        void GetProjection(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Matrix4x4* const matrix{ (Math::DX_Matrix4x4* const)data };
            assert(sizeof(Math::DX_Matrix4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.Projection());
        }

        void GetInverseProjection(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Matrix4x4* const matrix{ (Math::DX_Matrix4x4* const)data };
            assert(sizeof(Math::DX_Matrix4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.InverseProjection());
        }

        void GetViewProjection(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Matrix4x4* const matrix{ (Math::DX_Matrix4x4* const)data };
            assert(sizeof(Math::DX_Matrix4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.ViewProjection());
        }

        void GetInverseViewProjection(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Matrix4x4* const matrix{ (Math::DX_Matrix4x4* const)data };
            assert(sizeof(Math::DX_Matrix4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.InverseViewProjection());
        }

        void GetUpVector(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Math::DX_Vector3* const up_vector{ (Math::DX_Vector3* const)data };
            assert(sizeof(Math::DX_Vector3) == size);
            DirectX::XMStoreFloat3(up_vector, camera.UpVector());
        }

        constexpr void GetFieldOfView(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Perspective);
            float32* const fov{ (float32* const)data };
            assert(sizeof(float32) == size);
            *fov = camera.FieldOfView();
        }

        constexpr void GetAspectRatio(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Perspective);
            float32* const aspect_ratio{ (float32* const)data };
            assert(sizeof(float32) == size);
            *aspect_ratio = camera.AspectRatio();
        }

        constexpr void GetViewWidth(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Orthographic);
            float32* const view_width{ (float32* const)data };
            assert(sizeof(float32) == size);
            *view_width = camera.ViewWidth();
        }

        constexpr void GetViewHeight(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            assert(camera.RrojectionType() == Graphics::Camera::Type::Orthographic);
            float32* const view_height{ (float32* const)data };
            assert(sizeof(float32) == size);
            *view_height = camera.ViewHeight();
        }

        constexpr void GetNearZ(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            float32* const near_z{ (float32* const)data };
            assert(sizeof(float32) == size);
            *near_z = camera.NearZ();
        }

        constexpr void GetFarZ(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            float32* const far_z{ (float32* const)data };
            assert(sizeof(float32) == size);
            *far_z = camera.FarZ();
        }

        constexpr void GetProjectionType(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            Graphics::Camera::Type* const type{ (Graphics::Camera::Type* const)data };
            assert(sizeof(Graphics::Camera::Type) == size);
            *type = camera.RrojectionType();
        }

        constexpr void GetEntityID(const D3D12Camera& camera, OUT void* const data, uint32 size)
        {
            ID::IDType* const entity_id{ (ID::IDType* const)data };
            assert(sizeof(ID::IDType) == size);
            *entity_id = camera.EntityID();
        }

        constexpr void SetDummy(D3D12Camera&, const void* const, uint32)
        {}

        using SetFunc = void(*)(D3D12Camera&, const void* const, uint32);
        using GetFunc = void(*)(const D3D12Camera&, OUT void* const, uint32);
        constexpr SetFunc SetFunctions[]
        {
            SetUpVector,
            SetFieldOfView,
            SetAspectRatio,
            SetViewWidth,
            SetViewHeight,
            SetNearZ,
            SetFarZ,
            SetDummy,
            SetDummy,
            SetDummy,
            SetDummy,
            SetDummy,
            SetDummy,
            SetDummy,
        };

        static_assert(_countof(SetFunctions) == CameraParameter::Count);

        constexpr GetFunc GetFunctions[]
        {
            GetUpVector,
            GetFieldOfView,
            GetAspectRatio,
            GetViewWidth,
            GetViewHeight,
            GetNearZ,
            GetFarZ,
            GetView,
            GetProjection,
            GetInverseProjection,
            GetViewProjection,
            GetInverseViewProjection,
            GetProjectionType,
            GetEntityID,
        };

        static_assert(_countof(GetFunctions) == CameraParameter::Count);

    } // ñ≥ñºãÛä‘

    D3D12Camera::D3D12Camera(CameraInitInfo info)
        : _UpVector{ DirectX::XMLoadFloat3(&info.UpVector) },
        _NearZ{ info.NearZ }, _FarZ{ info.FarZ },
        _FieldOfView{ info.FieldOfView }, _AspectRatio{ info.AspectRatio },
        _ProjectionType{ info.Type }, _EntityID{ info.EntityID }, _UpdateFlag{ true }
    {
        assert(ID::IsValid(_EntityID));
        Update();
    }

    void D3D12Camera::Update()
    {
        GameEntity::Entity entity{ GameEntity::EntityID{_EntityID} };
        using namespace DirectX;
        Math::DX_Vector3 pos{ entity.GetTransformComponent().GetPosition() };
        Math::DX_Vector3 dir{ entity.GetTransformComponent().GetOrientation() };
        _Position = XMLoadFloat3(&pos);
        _Direction = XMLoadFloat3(&dir);
        _View = XMMatrixLookToRH(_Position, _Direction, _UpVector);

        if (_UpdateFlag)
        {
            // D3D12ÇÃå¸Ç´Ç∆çáÇÌÇπÇÈÇΩÇﬂÇ…nearZÇ∆farZÇì¸ÇÍë÷Ç¶ÇƒÇ¢ÇÈ
            _Projection = (_ProjectionType == Graphics::Camera::Type::Perspective)
                ? XMMatrixPerspectiveFovRH(_FieldOfView * XM_PI, _AspectRatio, _FarZ, _NearZ)
                : XMMatrixOrthographicRH(_ViewWidth, _ViewHeight, _FarZ, _NearZ);
            _InverseProjection = XMMatrixInverse(nullptr, _Projection);
            _UpdateFlag = false;
        }

        _ViewProjection = XMMatrixMultiply(_View, _Projection);
        _InverseViewProjection = XMMatrixInverse(nullptr, _ViewProjection);
    }

    void D3D12Camera::SetUpVector(Math::DX_Vector3 up)
    {
        _UpVector = DirectX::XMLoadFloat3(&up);
    }

    constexpr void D3D12Camera::SetFieldOfView(float32 fov)
    {
        assert(_ProjectionType == Graphics::Camera::Type::Perspective);
        _FieldOfView = fov;
        _UpdateFlag = true;
    }

    constexpr void D3D12Camera::SetAspectRatio(float32 aspectRatio)
    {
        assert(_ProjectionType == Graphics::Camera::Type::Perspective);
        _AspectRatio = aspectRatio;
        _UpdateFlag = true;
    }

    constexpr void D3D12Camera::SetViewWidth(float32 width)
    {
        assert(width);
        assert(_ProjectionType == Graphics::Camera::Type::Orthographic);
        _ViewWidth = width;
        _UpdateFlag = true;
    }

    constexpr void D3D12Camera::SetViewHeight(float32 height)
    {
        assert(height);
        assert(_ProjectionType == Graphics::Camera::Type::Orthographic);
        _ViewHeight = height;
        _UpdateFlag = true;
    }

    constexpr void D3D12Camera::SetNearZ(float32 nearZ)
    {
        _NearZ = nearZ;
        _UpdateFlag = true;
    }

    constexpr void D3D12Camera::SetFarZ(float32 farZ)
    {
        _FarZ = farZ;
        _UpdateFlag = true;
    }

    Graphics::Camera CreateCamera(CameraInitInfo info)
    {
        return Graphics::Camera{ CameraID{ Cameras.Add(info) } };
    }

    void RemoveCamera(CameraID id)
    {
        assert(ID::IsValid(id));
        Cameras.Remove(id);
    }

    void SetParameter(CameraID id, CameraParameter::Parameter parameter, const void* const data, uint32 dataSize)
    {
        assert(data && dataSize);
        D3D12Camera& camera{ GetCamera(id) };
        assert(parameter < CameraParameter::Count);
        if (parameter < CameraParameter::Count)
        {
            SetFunctions[parameter](camera, data, dataSize);
        }
    }

    void GetParameter(CameraID id, CameraParameter::Parameter parameter, OUT void* const data, uint32 dataSize)
    {
        assert(data && dataSize);
        D3D12Camera& camera{ GetCamera(id) };
        assert(parameter < CameraParameter::Count);
        if (parameter < CameraParameter::Count)
        {
            GetFunctions[parameter](camera, data, dataSize);
        }
    }

    D3D12Camera& GetCamera(CameraID id)
    {
        assert(ID::IsValid(id));
        return Cameras[id];
    }

}