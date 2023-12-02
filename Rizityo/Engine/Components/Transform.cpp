#include "Transform.h"

namespace Rizityo::Transform
{
	namespace
	{
		Utility::Vector<Math::DX_Vector3> Positions;
		Utility::Vector<Math::DX_Vector4> Rotations;
		Utility::Vector<Math::DX_Vector3> Orientations;
		Utility::Vector<Math::DX_Vector3> Scales;
		Utility::Vector<Math::DX_Matrix4x4> ToWorld;
		Utility::Vector<Math::DX_Matrix4x4> InvWorld;
		Utility::Vector<uint8> HasTransform;

		Utility::Vector<uint8> ChangesFromPreviousFrame;
		uint8 ReadWriteFlag;

	} // ïœêî

	namespace
	{
		Math::DX_Vector3 CalculateOrientation(Math::DX_Vector4 rotation)
		{
			using namespace DirectX;
			XMVECTOR rotationQuat{ XMLoadFloat4(&rotation) };
			XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
			Math::DX_Vector3 orientation;
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

		void SetPosition(TransformID id, const Math::DX_Vector3& position)
		{
			const uint32 index = ID::GetIndex(id);
			Positions[index] = position;
			HasTransform[index] = 0;
			ChangesFromPreviousFrame[index] |= ComponentFlags::Position;
		}

		void SetRotation(TransformID id, const Math::DX_Vector4& rotation_quaternion)
		{
			const uint32 index = ID::GetIndex(id);
			Rotations[index] = rotation_quaternion;
			Orientations[index] = CalculateOrientation(rotation_quaternion);
			HasTransform[index] = 0;
			ChangesFromPreviousFrame[index] |= ComponentFlags::Rotation;
		}

		void SetOrientation(TransformID, const Math::DX_Vector3&)
		{
		}

		void SetScale(TransformID id, const Math::DX_Vector3& scale)
		{
			const uint32 index = ID::GetIndex(id);
			Scales[index] = scale;
			HasTransform[index] = 0;
			ChangesFromPreviousFrame[index] |= ComponentFlags::Scale;
		}

	} // ä÷êî

	Transform::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		const ID::IDType index = ID::GetIndex(entity.ID());
		if (index < Positions.size())
		{
			Math::DX_Vector4 rotation{ info.Rotation };
			Positions[index] = Math::DX_Vector3{ info.Position };
			Rotations[index] = rotation;
			Orientations[index] = CalculateOrientation(rotation);
			Scales[index] = Math::DX_Vector3{ info.Scale };
			HasTransform[index] = 0;
			ChangesFromPreviousFrame[index] = (uint8)ComponentFlags::All;
		}
		else
		{
			assert(Positions.size() == index);
			Positions.emplace_back(info.Position);
			Rotations.emplace_back(info.Rotation);
			Orientations.emplace_back(CalculateOrientation(Math::DX_Vector4{ info.Rotation }));
			Scales.emplace_back(info.Scale);
			ToWorld.emplace_back();
			InvWorld.emplace_back();
			HasTransform.emplace_back((uint8)0);
			ChangesFromPreviousFrame.emplace_back((uint8)ComponentFlags::All);
		}

		return Component{ TransformID{entity.ID()} };
	}

	void RemoveComponent([[maybe_unused]] Transform::Component component)
	{
		assert(component.IsValid());
	}

	void GetTransformMatrices(const GameEntity::EntityID id, OUT Math::DX_Matrix4x4& world, OUT Math::DX_Matrix4x4& inverseWorld)
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

	void GetUpdatedComponentsFlags(const GameEntity::EntityID* const ids, uint32 count, OUT uint8* const flags)
	{
		assert(ids && count && flags);
		ReadWriteFlag = 1;

		for (uint32 i = 0; i < count; i++)
		{
			assert(GameEntity::Entity{ ids[i] }.IsValid());
			flags[i] = ChangesFromPreviousFrame[ID::GetIndex(ids[i])];
		}
	}

	void Update(const ComponentCache* const cache, uint32 count)
	{
		assert(cache && count);

		if (ReadWriteFlag)
		{
			memset(ChangesFromPreviousFrame.data(), 0, ChangesFromPreviousFrame.size());
			ReadWriteFlag = 0;
		}

		for (uint32 i = 0; i < count; i++)
		{
			const ComponentCache& c{ cache[i] };
			assert(Component{ c.ID }.IsValid());

			if (c.Flags & ComponentFlags::Rotation)
			{
				SetRotation(c.ID, c.Rotation);
			}

			if (c.Flags & ComponentFlags::Orientation)
			{
				SetOrientation(c.ID, c.Orientation);
			}

			if (c.Flags & ComponentFlags::Position)
			{
				SetPosition(c.ID, c.Position);
			}

			if (c.Flags & ComponentFlags::Scale)
			{
				SetScale(c.ID, c.Scale);
			}
		}
	}

	Math::Vector3 Component::GetPosition() const
	{
		assert(IsValid());
		return Math::Vector3{ Positions[ID::GetIndex(_ID)] };
	}

	Math::Vector4 Component::GetRotation() const
	{
		assert(IsValid());
		return Math::Vector4{Rotations[ID::GetIndex(_ID)]};
	}

	Math::Vector3 Component::GetOrientation() const
	{
		assert(IsValid());
		return Math::Vector3{Orientations[ID::GetIndex(_ID)]};
	}

	Math::Vector3 Component::GetScale() const
	{
		assert(IsValid());
		return Math::Vector3{Scales[ID::GetIndex(_ID)]};
	}
}