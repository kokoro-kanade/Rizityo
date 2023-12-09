#pragma once
#include "ComponentsCommonHeaders.h"

namespace Rizityo::Script
{
	struct InitInfo
	{
		Internal::ScriptCreateFunc CreateFunc;
	};

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity);
	void RemoveComponent(Script::Component component);
	void Update(float dt);

}