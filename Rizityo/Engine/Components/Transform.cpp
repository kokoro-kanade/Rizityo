#include "Transform.h"

namespace Rizityo::Transform
{
	namespace
	{
		Utility::Vector<Math::Vector3> positions;
		Utility::Vector<Math::Vector4> rotations;
		Utility::Vector<Math::Vector3> scales;
	}

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		const ID::IDType index = ID::GetIndex(entity.GetID());
		if (positions.size() > index)
		{
			positions[index] = Math::Vector3(info.Position);
			rotations[index] = Math::Vector4(info.Rotation);
			scales[index] = Math::Vector3(info.Scale);
		}
		else
		{
			assert(positions.size() == index);
			positions.emplace_back(info.Position);
			rotations.emplace_back(info.Rotation);
			scales.emplace_back(info.Scale);
		}

		// Why: なぜ毎回最後のインデックスを返しているのか
		return Component{ TransformID{(ID::IDType)positions.size() - 1} };
	}

	void RemoveComponent(Transform::Component component)
	{
		assert(component.IsValid());
	}

	Math::Vector3 Component::GetPosition() const
	{
		assert(IsValid());
		return positions[ID::GetIndex(ID)];
	}

	Math::Vector4 Component::GetRotation() const
	{
		assert(IsValid());
		return rotations[ID::GetIndex(ID)];
	}

	Math::Vector3 Component::GetScale() const
	{
		assert(IsValid());
		return scales[ID::GetIndex(ID)];
	}
}