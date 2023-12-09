#pragma once
#include "../Components/ComponentsCommonHeaders.h"

namespace Rizityo::Script
{
	class EntityScript;

	DEFINE_ID_TYPE(ScriptID);

	class Component final
	{
	public:
		constexpr explicit Component(ScriptID id) : _ID{ id } {}
		constexpr Component() : _ID{ ID::INVALID_ID } {}
		constexpr ScriptID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

		template<typename T>
		T* GetScript()
		{
			assert(IsValid());
			return dynamic_cast<T*>(GetEntityScript(_ID)); // TODO : dynamic_cast‚ÍÀs‚ÉŒ^î•ñ‚ğŒ©‚é‚Ì‚Å’x‚¢
		}

	private:
		ScriptID _ID;

		EntityScript* GetEntityScript(ScriptID id);
	};


}