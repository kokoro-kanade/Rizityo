#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Script
{
	struct InitInfo
	{
		Internal::ScriptCreateFunc CreateFunc;
	};

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: �Q�Ɠn������l�n���ɂ���
	void RemoveComponent(Script::Component component);


}