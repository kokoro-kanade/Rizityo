#pragma once
#include "ComponentsCommonHeaders.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	// ëOï˚êÈåæ
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
	
		Entity CreateGameEntity(const InitInfo& info); // TODO?: éQè∆ìnÇµÇ©ÇÁílìnÇµÇ…Ç∑ÇÈ
		void RemoveGameEnity(EntityID id);
		bool IsAlive(EntityID id);
	
	}
}