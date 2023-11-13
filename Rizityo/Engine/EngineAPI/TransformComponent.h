#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Transform
{
	DEFINE_ID_TYPE(TransformID);

	class Component final
	{
	public:
		constexpr explicit Component(TransformID id) : ID{ id } {}
		constexpr Component() : ID{ ID::INVALID_ID } {}
		constexpr TransformID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

		Math::Vector3 GetPosition() const;
		Math::Vector4 GetRotation() const;
		Math::Vector3 GetScale() const;

	private:
		TransformID ID;
	};


}