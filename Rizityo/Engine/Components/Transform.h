#pragma once
#include "ComponentsCommon.h"

namespace Rizityo::Transform
{
	struct InitInfo
	{
		float32 Position[3]{};
		float32 Rotation[4]{}; // �N�H�[�^�j�I��
		float32 Scale[3] = { 1.f, 1.f, 1.f };
	};

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity); // TODO?: �Q�Ɠn������l�n���ɂ���
	void RemoveComponent(Transform::Component component);
	void GetTransformMatrices(const GameEntity::EntityID id, OUT Math::Matrix4x4& world, OUT Math::Matrix4x4& inverseWorld);
}