#pragma once
#include "ComponentsCommon.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	// 前方宣言
	INIT_INFO(Transform);
	INIT_INFO(Script);

#undef INIT_INFO

	namespace GameEntity
	{
		struct EntityInfo // Rename: InitInfo
		{
			Transform::InitInfo* Transform{ nullptr };
			Script::InitInfo* Script{ nullptr };
		};
	
		Entity CreateGameEntity(const EntityInfo& info); // TODO?: 参照渡しから値渡しにする
		void RemoveGameEnity(EntityId id);
		bool IsAlive(EntityId id);
	
	}
}