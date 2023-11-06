#include "ContentLoader.h"
#include "../Components/Entity.h"
#include "../Components/Transform.h"
#include "../Components/Script.h"

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

	} // 無名空間

	bool LoadGame()
	{
		// ワーキングディレクトリを実行ファイルのパスに変更
		wchar_t path[MAX_PATH];
		const uint32 length = GetModuleFileName(0, &path[0], MAX_PATH);
		if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			return false;
		std::filesystem::path p{ path };
		SetCurrentDirectory(p.parent_path().wstring().c_str());

		// バイナリファイルを読み込んでエンティティを作成
		std::ifstream game("game.bin", std::ios::in | std::ios::binary);
		Utility::Vector<uint8> buffer(std::istreambuf_iterator<char>(game), {});
		assert(buffer.size());
		const uint8* at = buffer.data();
		constexpr uint32 shift = sizeof(uint32);
		const uint32 numEntities = *at; at += shift;
		if (!numEntities)
			return false;
		for (uint32 entityIndex = 0; entityIndex < numEntities; entityIndex++)
		{
			GameEntity::EntityInfo info{};
			const uint32 entityType = *at; at += shift;
			const uint32 numComponents = *at; at += shift;
			if (!numComponents)
				return false;
			for (uint32 componentIndex = 0; componentIndex < numComponents; componentIndex++)
			{
				const uint32 componentType = *at; at += shift;
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

		assert(at == buffer.data() + buffer.size());
		return true;
	}
	
	void UnLoadGame()
	{
		for (auto&& entity : entities)
		{
			GameEntity::RemoveGameEnity(entity.GetId());
		}
	}
}

#endif
