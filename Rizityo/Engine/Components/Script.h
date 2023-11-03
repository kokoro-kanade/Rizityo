#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Script
{
	struct InitInfo
	{
		Internal::ScriptCreateFunc CreateFunc;
	};

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: éQè∆ìnÇµÇ©ÇÁílìnÇµÇ…Ç∑ÇÈ
	void RemoveComponent(Script::Component component);


}