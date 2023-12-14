#include "CameraScript.h"

REGISTER_SCRIPT(CameraScript);

CameraScript::CameraScript(GameEntity::Entity entity)
    : Script::EntityScript{ entity }
{
    _InputSystem.AddHandler(Input::InputSource::Mouse, this, &CameraScript::MouseMove);

    const uint64 bindingKey = std::hash<std::string>()("Move");
    _InputSystem.AddHandler(bindingKey, this, &CameraScript::OnMove);

    _DesiredPosition = _Position = GetPosition();

    Math::Vector3 dir{ GetOrientation() };
    float32 theta = std::acosf(dir.y);
    float32 phi = std::atan2(-dir.z, dir.x);
    Math::Vector3 rot{ theta - Math::HALF_PI, phi + Math::HALF_PI, 0.f };
    _DesiredSpherical = _Spherical = rot;
}

void CameraScript::BeginPlay() {}

void CameraScript::Update(float32 dt)
{
    if (_MoveMagnitude > Math::EPSILON)
    {
        const float32 fps_scale{ dt / 0.016667f };
        Math::DX_Vector4 rot{ GetRotation() };
        Math::Vector3 d{ XMVector3Rotate(_Move * 0.05f * fps_scale, XMLoadFloat4(&rot)) };
        if (_PositionAcceleration < 1.f) _PositionAcceleration += (0.02f * fps_scale);
        _DesiredPosition += (d * _PositionAcceleration);
        _MovePosition = true;
    }
    else if (_MovePosition)
    {
        _PositionAcceleration = 0.f;
    }

    if (_MovePosition || _MoveRotation)
    {
        CameraSeek(dt);
    }
}

void CameraScript::OnMove(uint64 binding, const Input::InputValue& val)
{
    _Move = Math::Vector3(val.Current);
    _MoveMagnitude = _Move.LengthSquared();
}

void CameraScript::MouseMove(Input::InputSource::Type type, Input::InputCode::Code code, const Input::InputValue& mousePos)
{
    if (code == Input::InputCode::MousePosition)
    {
        Input::InputValue val;
        Input::GetInputValue(Input::InputSource::Mouse, Input::InputCode::MouseLeft, val);
        if (val.Current.z == 0.f)
            return;

        const float32 scale = 0.005f;
        const float32 dx = (mousePos.Current.x - mousePos.Previous.x) * scale;
        const float32 dy = (mousePos.Current.y - mousePos.Previous.y) * scale;

        Math::Vector3 spherical{ _DesiredSpherical };
        spherical.x += dy;
        spherical.y -= dx;
        spherical.x = Math::Clamp(spherical.x, 0.0001f - Math::HALF_PI, Math::HALF_PI - 0.0001f);

        _DesiredSpherical = spherical;
        _MoveRotation = true;
    }
}

void CameraScript::CameraSeek(float32 dt)
{
    Math::Vector3 p{ _DesiredPosition - _Position };
    Math::Vector3 o{ _DesiredSpherical - _Spherical };

    _MovePosition = (p.LengthSquared() > Math::EPSILON);
    _MoveRotation = (o.LengthSquared() > Math::EPSILON);

    const float32 scale = 0.2f * dt / 0.016667f;

    if (_MovePosition)
    {
        _Position += (p * scale);
        SetPosition(_Position);
    }

    if (_MoveRotation)
    {
        _Spherical += (o * scale);
        Math::Vector3 newRot{ _Spherical };
        newRot.x = Math::Clamp(newRot.x, 0.0001f - Math::HALF_PI, Math::HALF_PI - 0.0001f);
        _Spherical = newRot;

        Math::Quaternion rotQuat{ _Spherical };
        SetRotation(rotQuat);
    }
}