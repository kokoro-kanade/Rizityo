#pragma once
#include "../Components/ComponentsCommon.h"

namespace Rizityo::Script
{
	DEFINE_ID_TYPE(ScriptId);

	class Component final
	{
	public:
		constexpr explicit Component(ScriptId id) : Id{ id } {}
		constexpr Component() : Id{ Id::INVALID_ID } {}
		constexpr ScriptId GetId() const { return Id; }
		constexpr bool IsValid() const { return Id::IsValid(Id); }

	private:
		ScriptId Id;
	};


}