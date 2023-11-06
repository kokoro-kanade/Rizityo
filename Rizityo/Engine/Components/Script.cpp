#include "Script.h"
#include "Entity.h"

namespace Rizityo::Script
{
	namespace
	{
		Utility::Vector<Internal::ScriptPtr> EntityScripts; // �A���̈�ɕۑ�
		Utility::Vector<Id::IdType> IdMapping; // Component��index -> EntityScripts�̏ꏊ

		Utility::Vector<Id::GENERATION_TYPE> Generations;
		Utility::Vector<ScriptId> FreeIds;

		// Why: Entity��IsAlive�̂悤�Ƀw�b�_�[�Ɋ֐��錾���Ē�`����΂悢�̂ł͂Ȃ����H
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

		ScriptId id;
		if (FreeIds.size() > Id::MIN_DELETED_ELEMENTS) // FreeIds�����Ȃ���ԂŎg���܂킷�Ƃ�����generation��������Ă��܂��̂�臒l��݂���
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
		const ScriptId lastId{ (scriptEntityIndex != EntityScripts.size()-1) ? EntityScripts.back()->GetScriptComponent().GetId() : id };
		Utility::EraseUnordered(EntityScripts, scriptEntityIndex);
		IdMapping[Id::GetIndex(lastId)] = scriptEntityIndex; 
		IdMapping[Id::GetIndex(id)] = Id::INVALID_ID; // �v�f����̎���id == lastId�Ȃ̂�invalid_id�̑������
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
	const uint32 size = Rizityo::Script::ScriptNames().size();
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
