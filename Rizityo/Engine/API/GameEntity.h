#pragma once
#include "../Components/ComponentsCommonHeaders.h"
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

			[[nodiscard]] Math::Vector3 GetPosition() const { return GetTransformComponent().GetPosition(); }
			[[nodiscard]] Math::Quaternion GetRotation() const { return GetTransformComponent().GetRotation(); }
			[[nodiscard]] Math::Vector3 GetOrientation() const { return GetTransformComponent().GetOrientation(); }
			[[nodiscard]] Math::Vector3 GetScale() const { return GetTransformComponent().GetScale(); }

		private:

			EntityID _ID;
		};

		Entity Spawn(const char* entityName, const Math::Vector3& pos = {}, const Math::Vector3& rot = {});
	}

	namespace Script
	{
		class EntityScript : public GameEntity::Entity
		{
		public:

			virtual ~EntityScript() = default;
			virtual void BeginPlay() {}
			virtual void Update(float) {}

		protected:

			constexpr explicit EntityScript(GameEntity::Entity entity) : GameEntity::Entity{entity.ID()}{}

			void SetPosition(Math::Vector3 position) const { SetPosition(this, position); }
			void SetRotation(Math::Quaternion rotationQuaternion) const { SetRotation(this, rotationQuaternion); }
			void SetOrientation(Math::Vector3 orientationVector) const { SetOrientation(this, orientationVector); }
			void SetScale(Math::Vector3 scale) const { SetScale(this, scale); }

			// TODO? : 参照で受け取る
			static void SetPosition(const GameEntity::Entity* const entity, Math::Vector3 position);
			static void SetRotation(const GameEntity::Entity* const entity, Math::Quaternion rotationQuaternion);
			static void SetOrientation(const GameEntity::Entity* const entity, Math::Vector3 orientationVector);
			static void SetScale(const GameEntity::Entity* const entity, Math::Vector3 scale);

			template<typename T>
			static T* GetScript(const GameEntity::Entity* const entity)
			{
				return entity->GetScriptComponent().GetScript<T>();
			}
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