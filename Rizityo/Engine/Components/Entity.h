#pragma once
#include "ComponentsCommon.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	INIT_INFO(Transform); // ‘O•ûéŒ¾

#undef INIT_INFO

	namespace GameEntity
	{
		struct EntityInfo
		{
			Transform::InitInfo* Transform{ nullptr };
		};
	
		Entity CreateGameEntity(const EntityInfo& info);
		void RemoveGameEnity(Entity entity);
		bool IsAlive(Entity entity);
	
	}
}