#pragma once
#include "../Components/ComponentsCommonHeaders.h"

namespace Rizityo::Math
{
	class Vector3;
	class Vector4;
}

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

		[[nodiscard]] Math::Vector3 GetPosition() const;
		[[nodiscard]] Math::Quaternion GetRotation() const;
		[[nodiscard]] Math::Vector3 GetOrientation() const;
		[[nodiscard]] Math::Vector3 GetScale() const;

	private:
		TransformID _ID;
	};


}