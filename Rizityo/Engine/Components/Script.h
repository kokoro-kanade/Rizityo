#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Script
{
	struct InitInfo
	{
		Internal::ScriptCreateFunc CreateFunc;
	};

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: 参照渡しから値渡しにする
	void RemoveComponent(Script::Component component);
	void Update(float dt);

}