#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Script
{
	DEFINE_ID_TYPE(ScriptID);

	class Component final
	{
	public:
		constexpr explicit Component(ScriptID id) : _ID{ id } {}
		constexpr Component() : _ID{ ID::INVALID_ID } {}
		constexpr ScriptID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

	private:
		ScriptID _ID;
	};


}