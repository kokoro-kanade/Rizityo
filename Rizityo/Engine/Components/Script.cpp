#include "Script.h"
#include "Entity.h"

namespace Rizityo::Script
{
	namespace
	{
		Utility::Vector<Internal::ScriptPtr> EntityScripts; // 連続領域に保存
		Utility::Vector<ID::IDType> IdMapping; // Componentのindex -> EntityScriptsの場所

		Utility::Vector<ID::GENERATION_TYPE> Generations;
		Utility::Deque<ScriptID> FreeIds;

		// Why: EntityのIsAliveのようにヘッダーに関数宣言して定義すればよいのではないか？
		bool Exists(ScriptID id)
		{
			assert(ID::IsValid(id));
			const ID::IDType index{ ID::GetIndex(id) };
			assert(index < Generations.size() && IdMapping[index] < EntityScripts.size());
			return (Generations[index] == ID::GetGeneration(id) && EntityScripts[IdMapping[index]] && EntityScripts[IdMapping[index]]->IsValid());
		}

		using ScriptRegister = std::unordered_map<size_t, Internal::ScriptCreateFunc>;
		ScriptRegister& Register()
		{
			static ScriptRegister reg;
			return reg;
		}

#ifdef USE_EDITOR
		Utility::Vector<std::string>& ScriptNames()
		{
			static Utility::Vector<std::string> names;
			return names;
		}
#endif // USE_EDITOR

	}

	namespace Internal
	{
		uint8 RegisterScript(size_t tag, ScriptCreateFunc func)
		{
			bool result = Register().insert(ScriptRegister::value_type{ tag, func }).second;
			assert(result);
			return result;
		}

		ScriptCreateFunc GetScriptCreateFunc(size_t tag)
		{
			auto iter = Register().find(tag);
			assert(iter != Register().end() && iter->first == tag);
			return iter->second;
		}

#ifdef USE_EDITOR
		uint8 AddScriptName(const char* name)
		{
			ScriptNames().emplace_back(name);
			return true;
		}
#endif // USE_EDITOR

	}

	Script::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		assert(info.CreateFunc);

		ScriptID id;
		if (FreeIds.size() > ID::MIN_DELETED_ELEMENTS) // FreeIdsが少ない状態で使いまわすとすぐにgenerationが一周してしまうので閾値を設ける
		{
			id = FreeIds.front();
			assert(!Exists(id));
			FreeIds.pop_front();
			id = ScriptID{ ID::IncrementGeneration(id) };
			Generations[ID::GetIndex(id)]++;
		}
		else
		{
			id = ScriptID{ (ID::IDType)Generations.size() };
			Generations.push_back(0);
			IdMapping.emplace_back();
		}

		assert(ID::IsValid(id));
		EntityScripts.emplace_back(info.CreateFunc(entity));
		assert(EntityScripts.back()->GetID() == entity.GetID());
		const ID::IDType entityScriptIndex{ (ID::IDType)EntityScripts.size() - 1 };
		IdMapping[ID::GetIndex(id)] = entityScriptIndex;
		return Component{ id };
	}

	void RemoveComponent(Script::Component component)
	{
		assert(component.IsValid() && Exists(component.GetID()));
		const ScriptID id{ component.GetID() };
		const ID::IDType scriptEntityIndex{ IdMapping[ID::GetIndex(id)] };
		const ScriptID lastId{ (scriptEntityIndex != EntityScripts.size()-1) ? EntityScripts.back()->GetScriptComponent().GetID() : id };
		Utility::EraseUnordered(EntityScripts, scriptEntityIndex);
		IdMapping[ID::GetIndex(lastId)] = scriptEntityIndex; 
		IdMapping[ID::GetIndex(id)] = ID::INVALID_ID; // 要素が一つの時はid == lastIdなのでinvalid_idの代入が後
	}

	void Update(float dt)
	{
		for (auto& ptr : EntityScripts)
		{
			ptr->Update(dt);
		}
	}

} // Script

#ifdef USE_EDITOR
#include <atlsafe.h>

extern "C" __declspec(dllexport)
LPSAFEARRAY GetScriptNames()
{
	const uint32 size = (uint32)Rizityo::Script::ScriptNames().size();
	if (!size)
		return nullptr;
	CComSafeArray<BSTR> names(size);
	for (uint32 i = 0; i < size; i++)
	{
		names.SetAt(i, A2BSTR_EX(Rizityo::Script::ScriptNames()[i].c_str()), false);
	}
	return names.Detach();
}
#endif // USE_EDITOR
