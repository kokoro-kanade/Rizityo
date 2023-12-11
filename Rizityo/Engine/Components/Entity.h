#pragma once
#include "ComponentsCommonHeaders.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	// �O���錾
	INIT_INFO(Transform);
	INIT_INFO(Script);
	INIT_INFO(Render);

#undef INIT_INFO

	namespace GameEntity
	{
		struct InitInfo
		{
			Transform::InitInfo* Transform = nullptr;
			Script::InitInfo* Script = nullptr;
			Render::InitInfo* Render = nullptr;
		};
	
		Entity CreateGameEntity(const InitInfo& info); // TODO?: �Q�Ɠn������l�n���ɂ���
		void RemoveGameEnity(EntityID id);
		bool IsAlive(EntityID id);
	
	}
}