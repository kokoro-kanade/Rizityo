#include "Entity.h"
#include "Transform.h"
#include "Script.h"

namespace Rizityo::GameEntity
{
	namespace
	{
		Utility::Vector<Transform::Component> TransformComponents;
		Utility::Vector<Script::Component> ScriptComponents;

		Utility::Vector<Id::GENERATION_TYPE> Generations;
		Utility::Deque<EntityId> FreeIds;
	}

	Entity CreateGameEntity(const EntityInfo& info)
	{
		assert(info.Transform);
		if (!info.Transform)
			return Entity{};

		// ID取得
		EntityId id;
		if (FreeIds.size() > Id::MIN_DELETED_ELEMENTS) // FreeIdsが少ない状態で使いまわすとすぐにgenerationが一周してしまうので閾値を設ける
		{
			id = FreeIds.front();
			assert(!IsAlive(id));
			FreeIds.pop_front();
			id = EntityId{ Id::IncrementGeneration(id) };
			Generations[Id::GetIndex(id)]++;
		}
		else
		{
			id = EntityId{ (Id::IdType)Generations.size() };
			Generations.push_back(0);

			TransformComponents.emplace_back();
			ScriptComponents.emplace_back();
		}

		const Entity newEntity{ id };
		const Id::IdType index{ Id::GetIndex(id) };

		// TransformComponent作成
		assert(!TransformComponents[index].IsValid());
		TransformComponents[index] = Transform::CreateComponent(*info.Transform, newEntity);
		if (!TransformComponents[index].IsValid())
			return Entity{};

		// ScriptComponent作成
		if (info.Script && info.Script->CreateFunc)
		{
			assert(!ScriptComponents[index].IsValid());
			ScriptComponents[index] = Script::CreateComponent(*info.Script, newEntity);
			assert(ScriptComponents[index].IsValid());
		}

		return newEntity;
	}

	void RemoveGameEnity(EntityId id)
	{
		assert(IsAlive(id));
		Id::IdType index{ Id::GetIndex(id) };

		Transform::RemoveComponent(TransformComponents[index]);
		TransformComponents[index] = {};

		if (ScriptComponents[index].IsValid())
		{
			Script::RemoveComponent(ScriptComponents[index]);
			ScriptComponents[index] = {};
		}

		FreeIds.push_back(id);
	}

	bool IsAlive(EntityId id)
	{
		assert(Id::IsValid(id));
		Id::IdType index{ Id::GetIndex(id) };
		assert(index < Generations.size());
		return (Generations[index] == Id::GetGeneration(id) && TransformComponents[index].IsValid());
	}

	Transform::Component Entity::GetTransformComponent() const
	{
		assert(IsAlive(Id));
		const Id::IdType index{ Id::GetIndex(Id) };
		return TransformComponents[index];
	}

	Script::Component Entity::GetScriptComponent() const
	{
		assert(IsAlive(Id));
		const Id::IdType index{ Id::GetIndex(Id) };
		return ScriptComponents[index];
	}
}
