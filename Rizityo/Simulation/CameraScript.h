#pragma once
#include "API/GameEntity.h"
#include "API/Input.h"

using namespace Rizityo;

class CameraScript : public Script::EntityScript
{
public:
    explicit CameraScript(GameEntity::Entity entity);

    void BeginPlay() override;

    void Update(float32 dt) override;

private:

    Input::InputSystem<CameraScript>  _InputSystem{};

    Math::Vector3 _DesiredPosition;
    Math::Vector3 _DesiredSpherical;
    Math::Vector3 _Position;
    Math::Vector3 _Spherical;
    Math::Vector3 _Move{};
    float32 _MoveMagnitude = 0.f;
    float32 _PositionAcceleration = 0.f;
    bool _MovePosition = false;
    bool _MoveRotation = false;

    void OnMove(uint64 binding, const Input::InputValue& val);
    void MouseMove(Input::InputSource::Type type, Input::InputCode::Code code, const Input::InputValue& mousePos);
    void CameraSeek(float32 dt);
};