#include "Entity.h"
#include "Transform.h"
#include "Script.h"

namespace Rizityo::GameEntity
{
	namespace
	{
		Utility::Vector<Transform::Component> TransformComponents;
		Utility::Vector<Script::Component> ScriptComponents;

		Utility::Vector<ID::GENERATION_TYPE> Generations;
		Utility::Deque<EntityID> FreeIds;
	}

	Entity CreateGameEntity(const EntityInfo& info)
	{
		assert(info.Transform);
		if (!info.Transform)
			return Entity{};

		// ID
		EntityID id;
		if (FreeIds.size() > ID::MIN_DELETED_ELEMENTS) // 
		{
			id = FreeIds.front();
			assert(!IsAlive(id));
			FreeIds.pop_front();
			id = EntityID{ ID::IncrementGeneration(id) };
			Generations[ID::GetIndex(id)]++;
		}
		else
		{
			id = EntityID{ (ID::IDType)Generations.size() };
			Generations.push_back(0);

			TransformComponents.emplace_back();
			ScriptComponents.emplace_back();
		}

		const Entity newEntity{ id };
		const ID::IDType index{ ID::GetIndex(id) };

		// TransformComponent
		assert(!TransformComponents[index].IsValid());
		TransformComponents[index] = Transform::CreateComponent(*info.Transform, newEntity);
		if (!TransformComponents[index].IsValid())
			return Entity{};

		// ScriptComponent
		if (info.Script && info.Script->CreateFunc)
		{
			assert(!ScriptComponents[index].IsValid());
			ScriptComponents[index] = Script::CreateComponent(*info.Script, newEntity);
			assert(ScriptComponents[index].IsValid());
		}

		return newEntity;
	}

	void RemoveGameEnity(EntityID id)
	{
		assert(IsAlive(id));
		ID::IDType index{ ID::GetIndex(id) };

		Transform::RemoveComponent(TransformComponents[index]);
		TransformComponents[index] = {};

		if (ScriptComponents[index].IsValid())
		{
			Script::RemoveComponent(ScriptComponents[index]);
			ScriptComponents[index] = {};
		}

		FreeIds.push_back(id);
	}

	bool IsAlive(EntityID id)
	{
		assert(ID::IsValid(id));
		ID::IDType index{ ID::GetIndex(id) };
		assert(index < Generations.size());
		return (Generations[index] == ID::GetGeneration(id) && TransformComponents[index].IsValid());
	}

	Transform::Component Entity::GetTransformComponent() const
	{
		assert(IsAlive(_ID));
		const ID::IDType index{ ID::GetIndex(_ID) };
		return TransformComponents[index];
	}

	Script::Component Entity::GetScriptComponent() const
	{
		assert(IsAlive(_ID));
		const ID::IDType index{ ID::GetIndex(_ID) };
		return ScriptComponents[index];
	}
}
