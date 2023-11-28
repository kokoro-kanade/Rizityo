#pragma once
#include "../Components/ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace Rizityo
{
	namespace GameEntity
	{
		DEFINE_ID_TYPE(EntityID);

		class Entity
		{
		public:

			constexpr explicit Entity(EntityID id) : _ID{ id } {}
			constexpr Entity() : _ID{ ID::INVALID_ID } {}
			[[nodiscard]] constexpr EntityID ID() const { return _ID; }
			[[nodiscard]] constexpr bool IsValid() const { return ID::IsValid(_ID); }

			[[nodiscard]] Transform::Component GetTransformComponent() const;
			[[nodiscard]] Script::Component GetScriptComponent() const;

			[[nodiscard]] Math::Vector4 GetRotation() const { return GetTransformComponent().GetRotation(); }
			[[nodiscard]] Math::Vector3 GetOrientation() const { return GetTransformComponent().GetOrientation(); }
			[[nodiscard]] Math::Vector3 GetPosition() const { return GetTransformComponent().GetPosition(); }
			[[nodiscard]] Math::Vector3 GetScale() const { return GetTransformComponent().GetScale(); }

		private:

			EntityID _ID;
		};
	}

	// TODO: このヘッダーファイルに書くべきなのか
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

			constexpr explicit EntityScript(GameEntity::Entity entity) : GameEntity::Entity{entity.ID()}{}

			void SetPosition(Math::Vector3 position) const { SetPosition(this, position); }
			void SetRotation(Math::Vector4 rotation_quaternion) const { SetRotation(this, rotation_quaternion); }
			void SetOrientation(Math::Vector3 orientation_vector) const { SetOrientation(this, orientation_vector); }
			void SetScale(Math::Vector3 scale) const { SetScale(this, scale); }

			static void SetPosition(const GameEntity::Entity* const entity, Math::Vector3 position);
			static void SetRotation(const GameEntity::Entity* const entity, Math::Vector4 rotation_quaternion);
			static void SetOrientation(const GameEntity::Entity* const entity, Math::Vector3 orientation_vector);
			static void SetScale(const GameEntity::Entity* const entity, Math::Vector3 scale);
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