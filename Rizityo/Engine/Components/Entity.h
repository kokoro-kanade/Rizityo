#pragma once
#include "ComponentsCommonHeaders.h"

namespace Rizityo
{
#define INIT_INFO(Component) namespace Component { struct InitInfo; }

	// 前方宣言
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
	
		Entity CreateGameEntity(const InitInfo& info); // TODO?: 参照渡しから値渡しにする
		void RemoveGameEnity(EntityID id);
		bool IsAlive(EntityID id);
	
		namespace Internal
		{
			using StringHash = std::hash<std::string>;
			// エディタ側から使う想定
			void RegisterEntity(const char* entityName, GameEntity::InitInfo* info);
		}

	}
}