#include "Entity.h"
#include "Transform.h"

namespace Rizityo::GameEntity
{
	namespace
	{
		Utility::vector<Transform::Component> TransformComponents;

		Utility::vector<Id::GENERATION_TYPE> Generations;
		Utility::deque<EntityId> FreeIds;
	}

	Entity CreateGameEntity(const EntityInfo& info)
	{
		assert(info.Transform);
		if (!info.Transform)
			return Entity{};

		EntityId id;
		if (FreeIds.size() > Id::MIN_DELETED_ELEMENTS) // FreeIds‚ª­‚È‚¢ó‘Ô‚Åg‚¢‚Ü‚í‚·‚Æ‚·‚®‚Égeneration‚ªˆêü‚µ‚Ä‚µ‚Ü‚¤‚Ì‚Åè‡’l‚ğİ‚¯‚é
		{
			id = FreeIds.front();
			assert(!IsAlive(Entity{ id }));
			FreeIds.pop_front();
			id = EntityId{ Id::IncrementGeneration(id) };
			Generations[Id::GetIndex(id)]++;
		}
		else
		{
			id = EntityId{ (Id::IdType)Generations.size() };
			Generations.push_back(0);

			TransformComponents.emplace_back();
		}

		const Entity newEntity{ id };
		const Id::IdType index{ Id::GetIndex(id) };

		assert(!TransformComponents[index].IsValid());
		TransformComponents[index] = Transform::CreateTransformComponent(*info.Transform, newEntity);
		if (!TransformComponents[index].IsValid())
			return Entity{};
		return newEntity;
	}

	void RemoveGameEnity(Entity entity)
	{
		assert(IsAlive(entity));
		if (IsAlive(entity))
		{
			EntityId id{ entity.GetId() };
			Id::IdType index{ Id::GetIndex(id) };
			TransformComponents[index] = {};
			FreeIds.push_back(id);
		}
	}

	bool IsAlive(Entity entity)
	{
		assert(entity.IsValid());
		EntityId entityId = entity.GetId();
		Id::IdType index{ Id::GetIndex(entityId) };
		assert(index < Generations.size());
		return (Generations[index] == Id::GetGeneration(entityId) && TransformComponents[index].IsValid());
	}

	Transform::Component Entity::GetTransformComponent() const
	{
		assert(IsAlive(*this));
		const Id::IdType index{ Id::GetIndex(Id) };
		return TransformComponents[index];
	}
}
