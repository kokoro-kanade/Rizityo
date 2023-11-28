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

		Math::Vector3 GetPosition() const;
		Math::Vector4 GetRotation() const;
		Math::Vector3 GetOrientation() const;
		Math::Vector3 GetScale() const;

	private:
		TransformID _ID;
	};


}