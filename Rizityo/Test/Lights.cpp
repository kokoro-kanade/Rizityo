#include "EngineAPI/GameEntity.h"
#include "EngineAPI/Light.h"
#include "EngineAPI/TransformComponent.h"
#include "Graphics/Renderer.h"

using namespace Rizityo;

GameEntity::Entity CreateTestGameEntity(Math::Vector3 position, Math::Vector3 rotation, const char* scriptName);
void RemoveGameEntity(GameEntity::EntityID id);

namespace
{
    const uint64 LeftLightSetKey = 0;
    const uint64 RightLightSetKey = 1;

    Utility::Vector<Graphics::Light> Lights;

    constexpr Math::Vector3 RGB_ToColor(uint8 r, uint8 g, uint8 b) { return { r / 255.f, g / 255.f , b / 255.f }; }

} // –³–¼‹óŠÔ

void GenerateLights()
{
    // left light
    Graphics::LightInitInfo info{};
    info.EntityID = CreateTestGameEntity({}, { 0, 0, 0 }, nullptr).ID();
    info.Type = Graphics::Light::Directional;
    info.LightSetKey = LeftLightSetKey;
    info.Intensity = 1.f;
    info.Color = RGB_ToColor(174, 174, 174);

    Lights.emplace_back(Graphics::CreateLight(info));

    info.EntityID = CreateTestGameEntity({}, { Math::PI * 0.5f, 0, 0 }, nullptr).ID();
    info.Color = RGB_ToColor(17, 27, 48);
    Lights.emplace_back(Graphics::CreateLight(info));

    info.EntityID = CreateTestGameEntity({}, { -Math::PI * 0.5f, 0, 0 }, nullptr).ID();
    info.Color = RGB_ToColor(63, 47, 30);
    Lights.emplace_back(Graphics::CreateLight(info));

    // right light
    info.EntityID = CreateTestGameEntity({}, { 0, 0, 0 }, nullptr).ID();
    info.LightSetKey = RightLightSetKey;
    info.Color = RGB_ToColor(150, 100, 200);
    Lights.emplace_back(Graphics::CreateLight(info));

    info.EntityID = CreateTestGameEntity({}, { Math::PI * 0.5f, 0, 0 }, nullptr).ID();
    info.Color = RGB_ToColor(17, 27, 48);
    Lights.emplace_back(Graphics::CreateLight(info));

    info.EntityID = CreateTestGameEntity({}, { -Math::PI * 0.5f, 0, 0 }, nullptr).ID();
    info.Color = RGB_ToColor(63, 47, 30);
    Lights.emplace_back(Graphics::CreateLight(info));
}

void RemoveLights()
{
    for (auto& light : Lights)
    {
        const GameEntity::EntityID id{ light.GetEntityID() };
        Graphics::RemoveLight(light.ID(), light.LightSetKey());
        RemoveGameEntity(id);
    }

    Lights.clear();
}