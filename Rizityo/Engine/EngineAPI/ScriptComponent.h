#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Script
{
	DEFINE_ID_TYPE(ScriptID);

	class Component final
	{
	public:
		constexpr explicit Component(ScriptID id) : ID{ id } {}
		constexpr Component() : ID{ ID::INVALID_ID } {}
		constexpr ScriptID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

	private:
		ScriptID ID;
	};


}