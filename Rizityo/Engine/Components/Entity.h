#pragma once
#include "ComponentsCommon.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	// �O���錾
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
	
		Entity CreateGameEntity(const EntityInfo& info); // TODO?: �Q�Ɠn������l�n���ɂ���
		void RemoveGameEnity(EntityId id);
		bool IsAlive(EntityId id);
	
	}
}