#include "Transform.h"

namespace Rizityo::Transform
{
	namespace
	{
		Utility::Vector<Math::Vector3> Positions;
		Utility::Vector<Math::Vector4> Rotations;
		Utility::Vector<Math::Vector3> Orientations;
		Utility::Vector<Math::Vector3> Scales;
		Utility::Vector<Math::Matrix4x4> ToWorld;
		Utility::Vector<Math::Matrix4x4> InvWorld;
		Utility::Vector<uint8> HasTransform;

		Math::Vector3 CalculateOrientation(Math::Vector4 rotation)
		{
			using namespace DirectX;
			XMVECTOR rotationQuat{ XMLoadFloat4(&rotation) };
			XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
			Math::Vector3 orientation;
			XMStoreFloat3(&orientation, XMVector3Rotate(front, rotationQuat));
			return orientation;
		}

		void CalculateTransformMatrices(ID::IDType index)
		{
			assert(index < Rotations.size());
			assert(index < Positions.size());
			assert(index < Scales.size());

			using namespace DirectX;
			XMVECTOR r{ XMLoadFloat4(&Rotations[index]) };
			XMVECTOR t{ XMLoadFloat3(&Positions[index]) };
			XMVECTOR s{ XMLoadFloat3(&Scales[index]) };

			XMMATRIX world{ XMMatrixAffineTransformation(s, XMQuaternionIdentity(), r, t) };
			XMStoreFloat4x4(&ToWorld[index], world);

			world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			XMMATRIX inverseWorld{ XMMatrixInverse(nullptr, world) };
			XMStoreFloat4x4(&InvWorld[index], inverseWorld);

			HasTransform[index] = 1;
		}

	} // –³–¼‹óŠÔ

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		const ID::IDType index = ID::GetIndex(entity.ID());
		if (index < Positions.size())
		{
			Math::Vector4 rotation{ info.Rotation };
			Positions[index] = Math::Vector3{ info.Position };
			Rotations[index] =rotation;
			Orientations[index] = CalculateOrientation(rotation);
			Scales[index] = Math::Vector3{info.Scale};
			HasTransform[index] = 0;
		}
		else
		{
			assert(Positions.size() == index);
			Positions.emplace_back(info.Position);
			Rotations.emplace_back(info.Rotation);
			Orientations.emplace_back(CalculateOrientation(Math::Vector4{ info.Rotation }));
			Scales.emplace_back(info.Scale);
			ToWorld.emplace_back();
			InvWorld.emplace_back();
			HasTransform.emplace_back((uint8)0);
		}

		return Component{ TransformID{entity.ID()} };
	}

	void RemoveComponent([[maybe_unused]] Transform::Component component)
	{
		assert(component.IsValid());
	}

	void GetTransformMatrices(const GameEntity::EntityID id, OUT Math::Matrix4x4& world, OUT Math::Matrix4x4& inverseWorld)
	{
		assert(GameEntity::Entity{ id }.IsValid());

		const ID::IDType entityIndex{ ID::GetIndex(id) };
		if (!HasTransform[entityIndex])
		{
			CalculateTransformMatrices(entityIndex);
		}

		world = ToWorld[entityIndex];
		inverseWorld = InvWorld[entityIndex];
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