#include "Script.h"
#include "Entity.h"
#include "Transform.h"

#define USE_TRANSFORM_CACHE_MAP 0

namespace Rizityo::Script
{
	namespace
	{
		Utility::Vector<Internal::ScriptPtr> EntityScripts; // 連続領域に保存
		Utility::Vector<ID::IDType> IdMapping; // Componentのindex -> EntityScriptsの場所

		Utility::Vector<ID::GENERATION_TYPE> Generations;
		Utility::Deque<ScriptID> FreeIds;

		Utility::Vector<Transform::ComponentCache> TransformCache;

#if USE_TRANSFORM_CACHE_MAP
		std::unordered_map<ID::IDType, uint32> CacheMap;
#endif

		// TODO?: EntityのIsAliveのようにヘッダーに関数宣言して定義すればよいのではないか？ -> Existsをほかのファイルから使うかどうか
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

#if USE_TRANSFORM_CACHE_MAP
		Transform::ComponentCache* const GetCachePtr(const GameEntity::Entity* const entity)
		{
			assert(GameEntity::IsAlive((*entity).ID()));
			const Transform::TransformID id{ (*entity).GetTransformComponent().ID() };

			uint32 index{ UINT32_INVALID_NUM };
			auto pair = CacheMap.try_emplace(id, ID::INVALID_ID);

			if (pair.second)
			{
				index = (uint32)TransformCache.size();
				TransformCache.emplace_back();
				TransformCache.back().ID = id;
				CacheMap[id] = index;
			}
			else
			{
				index = CacheMap[id];
			}

			assert(index < TransformCache.size());
			return &TransformCache[index];
		}
#else
		Transform::ComponentCache* const GetCachePtr(const GameEntity::Entity* const entity)
		{
			assert(GameEntity::IsAlive((*entity).ID()));
			const Transform::TransformID id{ (*entity).GetTransformComponent().ID() };

			for (auto& cache : TransformCache)
			{
				if (cache.ID == id)
				{
					return &cache;
				}
			}

			// キャッシュがなければ
			TransformCache.emplace_back();
			TransformCache.back().ID = id;

			return &TransformCache.back();
		}
#endif

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
		if (FreeIds.size() > ID::MIN_DELETED_ELEMENTS) // FreeIdsが少ない状態で使いまわすとすぐにgenerationが一周してしまうのでしきい値を設ける
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
		assert(EntityScripts.back()->ID() == entity.ID());
		const ID::IDType entityScriptIndex{ (ID::IDType)EntityScripts.size() - 1 };
		IdMapping[ID::GetIndex(id)] = entityScriptIndex;
		return Component{ id };
	}

	void RemoveComponent(Script::Component component)
	{
		assert(component.IsValid() && Exists(component.ID()));
		const ScriptID id{ component.ID() };
		const ID::IDType scriptEntityIndex{ IdMapping[ID::GetIndex(id)] };
		const ScriptID lastId{ (scriptEntityIndex != EntityScripts.size() - 1) ? EntityScripts.back()->GetScriptComponent().ID() : id };
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

		if (TransformCache.size())
		{
			Transform::Update(TransformCache.data(), (uint32)TransformCache.size());
			TransformCache.clear();

#if USE_TRANSFORM_CACHE_MAP
			CacheMap.clear();
#endif
		}
	}

	void EntityScript::SetPosition(const GameEntity::Entity* const entity, Math::Vector3 position)
	{
		Transform::ComponentCache& cache{ *GetCachePtr(entity) };
		cache.Flags |= Transform::ComponentFlags::Position;
		cache.Position = position;
		// cache.Position = { position.x, position.y position.z };
	}

	void EntityScript::SetRotation(const GameEntity::Entity* const entity, Math::Quaternion rotationQuaternion)
	{
		Transform::ComponentCache& cache{ *GetCachePtr(entity) };
		cache.Flags |= Transform::ComponentFlags::Rotation;
		cache.Rotation = rotationQuaternion;
	}

	void EntityScript::SetOrientation(const GameEntity::Entity* const entity, Math::Vector3 orientationVector)
	{
		Transform::ComponentCache& cache{ *GetCachePtr(entity) };
		cache.Flags |= Transform::ComponentFlags::Orientation;
		cache.Orientation = orientationVector;
	}

	void EntityScript::SetScale(const GameEntity::Entity* const entity, Math::Vector3 scale)
	{
		Transform::ComponentCache& cache{ *GetCachePtr(entity) };
		cache.Flags |= Transform::ComponentFlags::Scale;
		cache.Scale = scale;
	}

	EntityScript* Script::Component::GetEntityScript(ScriptID id)
	{
		const ID::IDType index{ ID::GetIndex(id) };
		return EntityScripts[IdMapping[index]].get();
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
