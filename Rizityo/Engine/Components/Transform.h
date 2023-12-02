#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Transform
{
	struct InitInfo
	{
		float32 Position[3]{};
		float32 Rotation[4]{}; // クォータニオン
		float32 Scale[3] = { 1.f, 1.f, 1.f };
	};

    struct ComponentFlags {
        enum Flags : uint32 {
            Rotation = 0x01,
            Orientation = 0x02,
            Position = 0x04,
            Scale = 0x08,
            All = Rotation | Orientation | Position | Scale
        };
    };

    struct ComponentCache
    {
        Math::DX_Vector4 Rotation;
        Math::DX_Vector3 Orientation;
        Math::DX_Vector3 Position;
        Math::DX_Vector3 Scale;
        TransformID ID;
        uint32 Flags;
    };

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: 参照渡しから値渡しにする
	void RemoveComponent(Transform::Component component);
	void GetTransformMatrices(const GameEntity::EntityID id, OUT Math::DX_Matrix4x4& world, OUT Math::DX_Matrix4x4& inverseWorld);

    void GetUpdatedComponentsFlags(const GameEntity::EntityID* const ids, uint32 count, OUT uint8* const flags);
    void Update(const ComponentCache* const cache, uint32 count);
}