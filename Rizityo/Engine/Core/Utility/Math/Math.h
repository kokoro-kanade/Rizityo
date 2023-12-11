#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Math
{
	template<typename T>
	[[nodiscard]] constexpr T Clamp(T value, T min, T max)
	{
		return (value < min) ? min : ((value > max) ? max : value);
	}

	constexpr bool IsEqual(float32 a, float32 b, float32 eps = EPSILON)
	{
		float32 diff = a - b;
		if (diff < 0.f)
		{
			diff = -diff;
		}
		return diff < eps;
	}

	template<uint32 bits>
	[[nodiscard]] constexpr uint32 PackUnitFloat(float32 f)
	{
		static_assert(bits <= sizeof(uint32) * 8);
		assert(f >= 0.f && f <= 1.f);
		constexpr float32 intervals = (float32)(((uint32)1 << bits) - 1);
		return (uint32)(intervals * f + 0.5f);
	}

	template<uint32 bits>
	[[nodiscard]] constexpr float32 UnPackToUnitFloat(uint32 ui)
	{
		static_assert(bits <= sizeof(uint32) * 8);
		assert(ui < ((uint32)1 << bits));
		constexpr float32 intervals = (float32)(((uint32)1 << bits) - 1);
		return (float32)ui / intervals;
	}

	template<uint32 bits>
	[[nodiscard]] constexpr uint32 PackFloat(float32 f, float32 min, float32 max)
	{
		assert(min < max);
		assert(f >= min && f <= max);
		const float32 ratio = (f - min) / (max - min);
		return PackUnitFloat<bits>(ratio);
	}

	template<uint32 bits>
	[[nodiscard]] constexpr float32 UnPackToFloat(uint32 ui, float32 min, float32 max)
	{
		assert(min < max);
		return min + UnPackToUnitFloat<bits>(ui) * (max - min);
	}

	// size以上のalignmentの倍数に変換
	template<uint64 Alignment>
	[[nodiscard]] constexpr uint64 AlignSizeUp(uint64 size)
	{
		static_assert(Alignment, "アラインメントはゼロでない必要があります");
		constexpr uint64 mask{ Alignment - 1 };
		static_assert(!(Alignment & mask), "アラインメントは2のべき乗である必要があります");
		return ((size + mask) & ~mask);
	}

	// size以下のalignmentの倍数に変換
	template<uint64 Alignment>
	[[nodiscard]] constexpr uint64 AlignSizeDown(uint64 size)
	{
		static_assert(Alignment, "アラインメントはゼロでない必要があります");
		constexpr uint64 mask{ Alignment - 1 };
		static_assert(!(Alignment & mask), "アラインメントは2のべき乗である必要があります");
		return (size & ~mask);
	}

	// size以上のalignmentの倍数に変換
	[[nodiscard]] constexpr uint64 AlignSizeUp(uint64 size, uint64 alignment)
	{
		assert(alignment && "アラインメントはゼロでない必要があります");
		const uint64 mask{ alignment - 1 };
		assert(!(alignment & mask) && "アラインメントは2のべき乗である必要があります");
		return ((size + mask) & ~mask);
	}

	// size以下のalignmentの倍数に変換
	[[nodiscard]] constexpr uint64 AlignSizeDown(uint64 size, uint64 alignment)
	{
		assert(alignment && "アラインメントはゼロでない必要があります");
		const uint64 mask{ alignment - 1 };
		assert(!(alignment & mask) && "アラインメントは2のべき乗である必要があります");
		return (size & ~mask);
	}

	[[nodiscard]] constexpr uint64 CalcCRC32U64(const uint8* const data, uint64 size)
	{
		assert(size >= sizeof(uint64));
		uint64 crc = 0;
		const uint8* at = data;
		const uint8* const end = data + AlignSizeDown<sizeof(uint64)>(size);
		while (at < end)
		{
			crc = _mm_crc32_u64(crc, *((const uint64*)at));
			at += sizeof(uint64);
		}

		return crc;
	}
}
