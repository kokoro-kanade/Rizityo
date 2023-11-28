#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Graphics
{

    DEFINE_ID_TYPE(LightID);

    class Light
    {
    public:
        enum Type : uint32
        {
            Directional,
            Point,
            Spot,
            Count
        };

        constexpr explicit Light(LightID id, uint64 lightSetKey) : _LightSetKey{ lightSetKey }, _ID{ id } {}

        constexpr Light() = default;

        constexpr LightID ID() const { return _ID; }
        constexpr uint64 LightSetKey() const { return _LightSetKey; }
        constexpr bool IsValid() const { return ID::IsValid(_ID); }

        void SetEnabled(bool is_enabled) const;
        void SetIntensity(float32 intensity) const;
        void SetColor(Math::Vector3 color) const;

        bool IsEnabled() const;
        float32 GetIntensity() const;
        Math::Vector3 GetColor() const;
        Type GetLightType() const;
        ID::IDType GetEntityID() const;

    private:
        uint64 _LightSetKey = 0;
        LightID _ID{ ID::INVALID_ID };
    };
}