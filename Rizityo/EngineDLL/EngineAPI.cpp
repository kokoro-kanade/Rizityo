#ifndef EDITOR_INTERFACE
#define EDITOR_INTERFACE extern "C" __declspec(dllexport)
#endif // !EDITOR_INTERFACE

#include "CommonHeaders.h"
#include "Id.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/Transform.h"

using namespace Rizityo;

namespace // �G�f�B�^�ƃG���W���Ԃ��Ȃ�����
{
	struct TransformComponent
	{
		float32 Position[3];
		float32 Rotation[3]; // �I�C���[�p
		float32 Scale[3];

		Transform::InitInfo ToInitInfo()
		{
			using namespace DirectX;
			Transform::InitInfo info{};
			memcpy(&info.Position[0], &Position[0], sizeof(float32) * _countof(Position));
			// �I�C���[�p����N�H�[�^�j�I���ւ̕ϊ�
			XMFLOAT3A rot{ &Rotation[0] };
			XMVECTOR q{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQ{};
			XMStoreFloat4A(&rotQ, q);
			memcpy(&info.Rotation[0], &rotQ.x, sizeof(float32) * _countof(Rotation));
			memcpy(&info.Scale[0], &Scale[0], sizeof(float32) * _countof(Scale));
			return info;
		}
	};

	struct GameEntityDescriptor
	{
		TransformComponent Transform;
	};

	GameEntity::Entity EntityFromId(Id::IdType id)
	{
		return GameEntity::Entity{ GameEntity::EntityId{id} };
	}
}

EDITOR_INTERFACE
Id::IdType CreateGameEntity(GameEntityDescriptor* d)
{
	assert(d);
	GameEntityDescriptor& desc{*d};
	Transform::InitInfo transformInfo{ desc.Transform.ToInitInfo() };
	GameEntity::EntityInfo entityInfo{
		&transformInfo
	};
	return GameEntity::CreateGameEntity(entityInfo).GetId();
}

EDITOR_INTERFACE
void RemoveGameEntity(Id::IdType id)
{
	assert(Id::IsValid(id));
	GameEntity::RemoveGameEnity(EntityFromId(id));
}