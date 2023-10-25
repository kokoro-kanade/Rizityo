#pragma once
#include "../Components/ComponentsCommon.h"
#include "TransformComponent.h"

namespace Rizityo::GameEntity
{
	DEFINE_ID_TYPE(EntityId);

	class Entity
	{
	public:
		constexpr explicit Entity(EntityId id) : Id{ id } {}
		constexpr Entity() : Id{Id::INVALID_ID} {}
		constexpr EntityId GetId() const { return Id; }
		constexpr bool IsValid() const { return Id::IsValid(Id); }

		Transform::Component GetTransformComponent() const;
	private:
		EntityId Id;
	};
}