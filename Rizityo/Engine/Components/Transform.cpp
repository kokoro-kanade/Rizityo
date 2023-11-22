#include "Transform.h"

namespace Rizityo::Transform
{
	namespace
	{
		Utility::Vector<Math::Vector3> Positions;
		Utility::Vector<Math::Vector4> Rotations;
		Utility::Vector<Math::Vector3> Orientations;
		Utility::Vector<Math::Vector3> Scales;

		Math::Vector3 CalculateOrientation(Math::Vector4 rotation)
		{
			using namespace DirectX;
			XMVECTOR rotationQuat{ XMLoadFloat4(&rotation) };
			XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
			Math::Vector3 orientation;
			XMStoreFloat3(&orientation, XMVector3Rotate(front, rotationQuat));
			return orientation;
		}
	} // –³–¼‹óŠÔ

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		const ID::IDType index = ID::GetIndex(entity.ID());
		if (Positions.size() > index)
		{
			Math::Vector4 rotation{ info.Rotation };
			Positions[index] = Math::Vector3{ info.Position };
			Rotations[index] =rotation;
			Orientations[index] = CalculateOrientation(rotation);
			Scales[index] = Math::Vector3{info.Scale};
		}
		else
		{
			assert(Positions.size() == index);
			Positions.emplace_back(info.Position);
			Rotations.emplace_back(info.Rotation);
			Orientations.emplace_back(CalculateOrientation(Math::Vector4{ info.Rotation }));
			Scales.emplace_back(info.Scale);
		}

		return Component{ TransformID{entity.ID()} };
	}

	void RemoveComponent([[maybe_unused]] Transform::Component component)
	{
		assert(component.IsValid());
	}

	Math::Vector3 Component::Position() const
	{
		assert(IsValid());
		return Positions[ID::GetIndex(_ID)];
	}

	Math::Vector4 Component::Rotation() const
	{
		assert(IsValid());
		return Rotations[ID::GetIndex(_ID)];
	}

	Math::Vector3 Component::Orientation() const
	{
		assert(IsValid());
		return Orientations[ID::GetIndex(_ID)];
	}

	Math::Vector3 Component::Scale() const
	{
		assert(IsValid());
		return Scales[ID::GetIndex(_ID)];
	}
}