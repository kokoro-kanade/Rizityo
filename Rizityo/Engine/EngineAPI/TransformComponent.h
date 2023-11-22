#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Transform
{
	DEFINE_ID_TYPE(TransformID);

	class Component final
	{
	public:
		constexpr explicit Component(TransformID id) : _ID{ id } {}
		constexpr Component() : _ID{ ID::INVALID_ID } {}
		constexpr TransformID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

		Math::Vector3 Position() const;
		Math::Vector4 Rotation() const;
		Math::Vector3 Orientation() const;
		Math::Vector3 Scale() const;

	private:
		TransformID _ID;
	};


}