#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Transform
{
	struct InitInfo
	{
		float32 Position[3]{};
		float32 Rotation[4]{};
		float32 Scale[3]{ 1.f, 1.f, 1.f };
	};

	Transform::Component CreateTransformComponent(const InitInfo& info, GameEntity::Entity entity);
	void RemoveTransformComponent(Transform::Component component);
}