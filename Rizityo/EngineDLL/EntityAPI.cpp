#include "Common.h"
#include "CommonHeaders.h"
#include "Id.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Script.h"

using namespace Rizityo;

namespace // エディタとエンジン間をつなぐ処理
{
	struct TransformComponent
	{
		float32 Position[3];
		float32 Rotation[3]; // オイラー角
		float32 Scale[3];

		Transform::InitInfo ToInitInfo()
		{
			using namespace DirectX;
			Transform::InitInfo info{};

			memcpy(&info.Position[0], &Position[0], sizeof(Position));

			// オイラー角からクォータニオンへの変換
			XMFLOAT3A rot{ &Rotation[0] };
			XMVECTOR q{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQ{};
			XMStoreFloat4A(&rotQ, q);
			memcpy(&info.Rotation[0], &rotQ.x, sizeof(info.Rotation));

			memcpy(&info.Scale[0], &Scale[0], sizeof(Scale));

			return info;
		}
	};

	struct ScriptComponent
	{
		Script::Internal::ScriptCreateFunc ScriptCreateFunc;

		Script::InitInfo ToInitInfo()
		{
			Script::InitInfo info{};
			info.CreateFunc = ScriptCreateFunc;
			return info;
		}
	};

	struct GameEntityDescriptor
	{
		TransformComponent Transform;
		ScriptComponent Script;
	};

	GameEntity::Entity EntityFromId(ID::IDType id)
	{
		return GameEntity::Entity{ GameEntity::EntityID{id} };
	}
}

EDITOR_INTERFACE
ID::IDType CreateGameEntity(GameEntityDescriptor* d)
{
	assert(d);
	GameEntityDescriptor& desc{ *d };
	Transform::InitInfo transformInfo{ desc.Transform.ToInitInfo() };
	Script::InitInfo scriptInfo{ desc.Script.ToInitInfo() };
	GameEntity::EntityInfo entityInfo{
		&transformInfo,
		&scriptInfo
	};
	return GameEntity::CreateGameEntity(entityInfo).ID();
}

EDITOR_INTERFACE
void RemoveGameEntity(ID::IDType id)
{
	assert(ID::IsValid(id));
	GameEntity::RemoveGameEnity(GameEntity::EntityID{ id });
}