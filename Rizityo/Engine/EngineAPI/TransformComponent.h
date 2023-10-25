#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Transform
{
	DEFINE_ID_TYPE(TransformId);

	class Component final
	{
	public:
		constexpr explicit Component(TransformId id) : Id{ id } {}
		constexpr Component() : Id{ Id::INVALID_ID } {}
		constexpr TransformId GetId() const { return Id; }
		constexpr bool IsValid() const { return Id::IsValid(Id); }

		Math::Vector3 GetPosition() const;
		Math::Vector4 GetRotation() const;
		Math::Vector3 GetScale() const;

	private:
		TransformId Id;
	};


}