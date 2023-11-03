#include "Script.h"
#include "Entity.h"

namespace Rizityo::Script
{
	namespace
	{
		Utility::Vector<Internal::ScriptPtr> EntityScripts; // 連続領域に保存
		Utility::Vector<Id::IdType> IdMapping; // Componentのindex -> EntityScriptsの場所

		Utility::Vector<Id::GENERATION_TYPE> Generations;
		Utility::Vector<ScriptId> FreeIds;

		// Why: EntityのIsAliveのようにヘッダーに関数宣言して定義すればよいのではないか？
		bool Exists(ScriptId id)
		{
			assert(Id::IsValid(id));
			const Id::IdType index{ Id::GetIndex(id) };
			assert(index < Generations.size() && IdMapping[index] < EntityScripts.size());
			return (Generations[index] == Id::GetGeneration(id) && EntityScripts[IdMapping[index]] && EntityScripts[IdMapping[index]]->IsValid());
		}

		using ScriptRegister = std::unordered_map<size_t, Internal::ScriptCreateFunc>;
		ScriptRegister& Register()
		{
			static ScriptRegister reg;
			return reg;
		}
	}

	namespace Internal
	{
		uint8 RegisterScript(size_t tag, ScriptCreateFunc func)
		{
			bool result = Register().insert(ScriptRegister::value_type{ tag, func }).second;
			assert(result);
			return result;
		}
	}

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		assert(info.CreateFunc);

		ScriptId id;
		if (FreeIds.size() > Id::MIN_DELETED_ELEMENTS) // FreeIdsが少ない状態で使いまわすとすぐにgenerationが一周してしまうので閾値を設ける
		{
			id = FreeIds.front();
			assert(!Exists(id));
			FreeIds.pop_back();
			id = ScriptId{ Id::IncrementGeneration(id) };
			Generations[Id::GetIndex(id)]++;
		}
		else
		{
			id = ScriptId{ (Id::IdType)Generations.size() };
			Generations.push_back(0);
			IdMapping.emplace_back();
		}

		assert(Id::IsValid(id));
		EntityScripts.emplace_back(info.CreateFunc(entity));
		assert(EntityScripts.back()->GetId() == entity.GetId());
		const Id::IdType entityScriptIndex{ (Id::IdType)EntityScripts.size() - 1 };
		IdMapping[Id::GetIndex(id)] = entityScriptIndex;
		return Component{ id };
	}

	void RemoveComponent(Script::Component component)
	{
		assert(component.IsValid() && Exists(component.GetId()));
		const ScriptId id{ component.GetId() };
		const Id::IdType scriptEntityIndex{ IdMapping[Id::GetIndex(id)] };
		const ScriptId lastId{ EntityScripts.back()->GetScriptComponent().GetId() };
		Utility::EraseUnordered(EntityScripts, scriptEntityIndex);
		IdMapping[Id::GetIndex(lastId)] = scriptEntityIndex; 
		IdMapping[Id::GetIndex(id)] = Id::INVALID_ID; // 要素が一つの時はid == lastIdなのでinvalid_idの代入が後
	}
}