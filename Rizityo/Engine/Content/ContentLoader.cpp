#include "ContentLoader.h"
#include "Components/Entity.h"
#include "Components/Transform.h"
#include "Components/Script.h"
#include "Graphics/Renderer.h"

#if !defined(SHIPPING)

#include <fstream>
#include <filesystem>
#include <Windows.h>

namespace Rizityo::Content
{
	namespace
	{
		enum ComponentType
		{
			Transform,
			Script,
			Count
		};

		Utility::Vector<GameEntity::Entity> entities;

		// infoはポインタを持つので関数のローカル変数ではなくてここで定義した変数を渡す
		Transform::InitInfo TransformInfo{};
		Script::InitInfo ScriptInfo{};
	}

	namespace
	{
		bool ReadTransform(const uint8*& data, GameEntity::EntityInfo& info)
		{
			using namespace DirectX;
			float32 rotation[3];

			assert(!info.Transform);
			memcpy(&TransformInfo.Position[0], data, sizeof(TransformInfo.Position)); data += sizeof(TransformInfo.Position);
			memcpy(&rotation[0], data, sizeof(rotation)); data += sizeof(rotation);
			memcpy(&TransformInfo.Scale[0], data, sizeof(TransformInfo.Scale)); data += sizeof(TransformInfo.Scale);

			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQuat{};
			XMStoreFloat4A( &rotQuat, quat );
			memcpy(&TransformInfo.Rotation[0], &rotQuat.x, sizeof(TransformInfo.Rotation));

			info.Transform = &TransformInfo;
			return true;
		}

		bool ReadScript(const uint8*& data, GameEntity::EntityInfo& info)
		{
			assert(!info.Script);
			const uint32 nameLength = *data; data += sizeof(uint32);
			if (!nameLength)
				return false;

			assert(nameLength < 256);
			char scriptName[256];
			memcpy(&scriptName, data, nameLength); data += nameLength;
			// cの文字列として終端を0にする
			scriptName[nameLength] = 0;
			ScriptInfo.CreateFunc = Script::Internal::GetScriptCreateFunc(Script::Internal::StringHash()(scriptName));

			info.Script = &ScriptInfo;
			return ScriptInfo.CreateFunc != nullptr;
		}

		using ComponentReader = bool(*)(const uint8*&, GameEntity::EntityInfo&);
		ComponentReader componentReaders[]
		{
			ReadTransform,
			ReadScript
		};

		static_assert(_countof(componentReaders) == ComponentType::Count);

		bool ReadFile(std::filesystem::path path, OUT std::unique_ptr<uint8[]>& data, OUT uint64& size)
		{
			if (!std::filesystem::exists(path))
				return false;

			size = std::filesystem::file_size(path);
			assert(size);
			if (!size)
				return false;

			data = std::make_unique<uint8[]>(size);
			std::ifstream file{ path, std::ios::in | std::ios::binary };
			if (!file || !file.read((char*)data.get(), size))
			{
				file.close();
				return false;
			}

			file.close();
			return true;
		}

	} // 無名空間

	bool LoadGame()
	{
		// バイナリファイルを読み込んでエンティティを作成
		std::unique_ptr<uint8[]> gameData{};
		uint64 size = 0;
		if (!ReadFile("game.bin", gameData, size))
			return false;
		assert(gameData.get());
		const uint8* at = gameData.get();
		constexpr uint32 su32 = sizeof(uint32);
		const uint32 numEntities = *at; at += su32;
		if (!numEntities)
			return false;
		for (uint32 entityIndex = 0; entityIndex < numEntities; entityIndex++)
		{
			GameEntity::EntityInfo info{};
			const uint32 entityType = *at; at += su32;
			const uint32 numComponents = *at; at += su32;
			if (!numComponents)
				return false;
			for (uint32 componentIndex = 0; componentIndex < numComponents; componentIndex++)
			{
				const uint32 componentType = *at; at += su32;
				assert(componentType < ComponentType::Count);
				if (!componentReaders[componentType](at, info))
					return false;
			}

			assert(info.Transform);
			GameEntity::Entity entity{ GameEntity::CreateGameEntity(info) };
			if (!entity.IsValid())
				return false;
			entities.emplace_back(entity);
		}

		assert(at == gameData.get() + size);
		return true;
	}
	
	void UnLoadGame()
	{
		for (auto&& entity : entities)
		{
			GameEntity::RemoveGameEnity(entity.GetID());
		}
	}

	bool LoadEngineShaders(std::unique_ptr<uint8[]>& shaders, uint64& size)
	{
		auto path = Graphics::GetEngineShadersPath();
		return ReadFile(path, shaders, size);
	}
}

#endif
