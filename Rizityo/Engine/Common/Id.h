#pragma once
#include "CommonHeaders.h"

namespace Rizityo::ID 
{
	using IDType = uint32;

	namespace Internal
	{
		constexpr uint32 GENERATION_BITS{ 8 };
		constexpr uint32 INDEX_BITS{ sizeof(IDType) * 8 - GENERATION_BITS };
		constexpr IDType GENERATION_MASK{ (1 << GENERATION_BITS) - 1 };
		constexpr IDType INDEX_MASK{ (1 << INDEX_BITS) - 1 };
	}

	constexpr IDType INVALID_ID{ (IDType)-1 };
	constexpr uint32 MIN_DELETED_ELEMENTS{ 1024 };

	using GENERATION_TYPE = std::conditional_t<Internal::GENERATION_BITS <= 16, std::conditional_t<Internal::GENERATION_BITS <= 8, uint8, uint16>, uint32>;
	static_assert(sizeof(GENERATION_TYPE) * 8 >= Internal::GENERATION_BITS);
	static_assert(sizeof(IDType) > sizeof(GENERATION_TYPE));

	constexpr bool IsValid(IDType id)
	{
		return id != INVALID_ID;
	}

	constexpr IDType GetIndex(IDType id)
	{
		IDType index = id & Internal::INDEX_MASK;
		assert(index != Internal::INDEX_MASK);
		return index;
	}

	constexpr IDType GetGeneration(IDType id)
	{
		return (id >> Internal::INDEX_BITS);
	}

	constexpr IDType IncrementGeneration(IDType id)
	{
		const IDType newGeneration{ GetGeneration(id) + 1 };
		assert(newGeneration < Internal::GENERATION_MASK);
		return GetIndex(id) | newGeneration << Internal::INDEX_BITS;
	}

#if _DEBUG
namespace Internal
{
	struct IDBase
	{
		constexpr explicit IDBase(IDType id) : ID{ id } {}
		constexpr operator IDType() const { return ID; }
	private:
		IDType ID;
	};
}

#define DEFINE_ID_TYPE(Name)												\
		struct Name final : ID::Internal::IDBase							\
		{																	\
			constexpr explicit Name(ID::IDType id)							\
				: IDBase{ id } {}											\
			constexpr Name() : IDBase { 0 }	{}  							\
		};

#else
#define DEFINE_ID_TYPE(Name) using Name = ID::IDType;

#endif

}