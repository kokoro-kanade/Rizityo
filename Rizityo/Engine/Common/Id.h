#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Id 
{
	using IdType = uint32;

	namespace Internal
	{
		constexpr uint32 GENERATION_BITS{ 8 };
		constexpr uint32 INDEX_BITS{ sizeof(IdType) * 8 - GENERATION_BITS };
		constexpr IdType GENERATION_MASK{ (1 << GENERATION_BITS) - 1 };
		constexpr IdType INDEX_MASK{ (1 << INDEX_BITS) - 1 };
	}

	constexpr IdType INVALID_ID{ (IdType)-1 };
	constexpr uint32 MIN_DELETED_ELEMENTS{ 1024 };

	using GENERATION_TYPE = std::conditional_t<Internal::GENERATION_BITS <= 16, std::conditional_t<Internal::GENERATION_BITS <= 8, uint8, uint16>, uint32>;
	static_assert(sizeof(GENERATION_TYPE) * 8 >= Internal::GENERATION_BITS);
	static_assert(sizeof(IdType) > sizeof(GENERATION_TYPE));

	constexpr bool IsValid(IdType id)
	{
		return id != INVALID_ID;
	}

	constexpr IdType GetIndex(IdType id)
	{
		IdType index = id & Internal::INDEX_MASK;
		assert(index != Internal::INDEX_MASK);
		return index;
	}

	constexpr IdType GetGeneration(IdType id)
	{
		return (id >> Internal::INDEX_BITS) & id;
	}

	constexpr IdType IncrementGeneration(IdType id)
	{
		const IdType newGeneration{ GetGeneration(id) + 1 };
		assert(newGeneration < Internal::GENERATION_MASK);
		return GetIndex(id) | newGeneration << Internal::INDEX_BITS;
	}

#if _DEBUG
namespace Internal
{
	struct IdBase
	{
		constexpr explicit IdBase(IdType id) : _id{ id } {}
		constexpr operator IdType() const { return _id; }
	private:
		IdType _id;
	};
}

#define DEFINE_ID_TYPE(Name)												\
		struct Name final : Id::Internal::IdBase							\
		{																	\
			constexpr explicit Name(Id::IdType id)							\
				: IdBase{ id } {}											\
			constexpr Name() : IdBase { 0 }	{}  							\
		};

#else
#define DEFINE_ID_TYPE(Name) using Name = Id::IdType;

#endif

}