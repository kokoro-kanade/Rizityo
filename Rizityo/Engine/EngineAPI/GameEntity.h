#pragma once
#include "..\Components\ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace Rizityo
{
	namespace GameEntity
	{
		DEFINE_ID_TYPE(EntityId);

		class Entity
		{
		public:
			constexpr explicit Entity(EntityId id) : Id{ id } {}
			constexpr Entity() : Id{ Id::INVALID_ID } {}
			constexpr EntityId GetId() const { return Id; }
			constexpr bool IsValid() const { return Id::IsValid(Id); }

			Transform::Component GetTransformComponent() const;
			Script::Component GetScriptComponent() const;
		private:
			EntityId Id;
		};
	}

	// TODO: このヘッダーファイルに書くべきなのか
	// Rename: Script -> Frameworkとか？(unrealっぽい)
	namespace Script
	{
		// Rename: unrealに対応するもの(beginplayとupdateを持つもの)を調べる(Actor?) or ScriptEntity 
		class EntityScript : public GameEntity::Entity
		{
		public:
			virtual ~EntityScript() = default;
			virtual void BeginPlay() {}
			virtual void Update(float) {}

		protected:
			constexpr explicit EntityScript(GameEntity::Entity entity) : GameEntity::Entity{entity.GetId()}{}
		};

		namespace Internal
		{
			using ScriptPtr = std::unique_ptr<EntityScript>;
			using ScriptCreateFunc = ScriptPtr(*)(GameEntity::Entity entity);
			using StringHash = std::hash<std::string>;

			uint8 RegisterScript(size_t, ScriptCreateFunc); // エンジン側にスクリプトクラスのハッシュと生成関数を登録

#ifdef USE_EDITOR
			extern "C" __declspec(dllexport)
#endif // USE_EDITOR
			ScriptCreateFunc GetScriptCreateFunc(size_t tag);

			template<class ScriptClass>
			ScriptPtr CreateScript(GameEntity::Entity entity)
			{
				assert(entity.IsValid());
				return std::make_unique<ScriptClass>(entity);
			}

#ifdef USE_EDITOR
			uint8 AddScriptName(const char* name);

#define REGISTER_SCRIPT(TYPE)												      \
			namespace															  \
			{																	  \
				const uint8 _Reg_##TYPE										      \
					= Rizityo::Script::Internal::RegisterScript(				  \
						Rizityo::Script::Internal::StringHash()(#TYPE),           \
						&Rizityo::Script::Internal::CreateScript<TYPE>);		  \
				const uint8 _Name_##TYPE                                          \
					= Rizityo::Script::Internal::AddScriptName(#TYPE);			  \
			}                                                                     

#else

#define REGISTER_SCRIPT(TYPE)												      \
			namespace															  \
			{																	  \
				const uint8 _Reg_##TYPE										      \
					= Rizityo::Script::Internal::RegisterScript(				  \
						Rizityo::Script::Internal::StringHash()(#TYPE),           \
						&Rizityo::Script::Internal::CreateScript<TYPE>);		  \
			}

#endif // USE_EDITOR

		} // Internal
	} // Script
}// Rizityo