#pragma once
#include "../Components/ComponentsCommonHeaders.h"

namespace Rizityo::Render
{
	DEFINE_ID_TYPE(RenderID);

	class Component final
	{
	public:
		constexpr explicit Component(RenderID id) : _ID{ id } {}
		constexpr Component() : _ID{ ID::INVALID_ID } {}
		constexpr RenderID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

	private:
		RenderID _ID;
	};
}