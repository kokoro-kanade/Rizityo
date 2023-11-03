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

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: 参照渡しから値渡しにする
	void RemoveComponent(Transform::Component component);
}