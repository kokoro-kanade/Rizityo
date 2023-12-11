#include "Entity.h"
#include "Transform.h"
#include "Script.h"
#include "Render.h"

namespace Rizityo::GameEntity
{
	namespace
	{
		Vector<Transform::Component> TransformComponents;
		Vector<Script::Component> ScriptComponents;
		Vector<Render::Component> RenderComponents;

		Vector<ID::GENERATION_TYPE> Generations;
		Deque<EntityID> FreeIds;
	}

	Entity CreateGameEntity(const InitInfo& info)
	{
		assert(info.Transform);
		if (!info.Transform)
			return Entity{};

		// ID
		EntityID id;
		if (FreeIds.size() > ID::MIN_DELETED_ELEMENTS) // FreeIds‚ª­‚È‚¢ó‘Ô‚ÅŽg‚¢‚Ü‚í‚·‚Æ‚·‚®‚Égeneration‚ªˆêŽü‚µ‚Ä‚µ‚Ü‚¤‚Ì‚Å‚µ‚«‚¢’l‚ðÝ‚¯‚é
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
			RenderComponents.emplace_back();
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

		// RendererComponent
		if (info.Render)
		{
			assert(!RenderComponents[index].IsValid());
			RenderComponents[index] = Render::CreateComponent(*info.Render, newEntity);
			assert(RenderComponents[index].IsValid());
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

		if (RenderComponents[index].IsValid())
		{
			Render::RemoveComponent(RenderComponents[index]);
			RenderComponents[index] = {};
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
