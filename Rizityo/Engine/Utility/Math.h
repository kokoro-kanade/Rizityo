#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Math
{
	template<typename T>
	constexpr T Clamp(T value, T min, T max)
	{
		return (value < min) ? min : ((value > max) ? max : value);
	}

	template<uint32 bits>
	constexpr uint32 PackUnitFloat(float32 f)
	{
		static_assert(bits <= sizeof(uint32) * 8);
		assert(f >= 0.f && f <= 1.f);
		constexpr float32 intervals = (float32)((1ui32 << bits) - 1);
		return (uint32)(intervals * f + 0.5f);
	}

	template<uint32 bits>
	constexpr float32 UnPackToUnitFloat(uint32 ui)
	{
		static_assert(bits <= sizeof(uint32) * 8);
		assert(ui < (1ui32 << bits));
		constexpr float32 intervals = (float32)((1ui32 << bits) - 1);
		return (float32)ui / intervals;
	}

	template<uint32 bits>
	constexpr uint32 PackFloat(float32 f, float32 min, float32 max)
	{
		assert(min < max);
		assert(f >= min && f <= max);
		const float32 ratio = (f - min) / (max - min);
		return PackUnitFloat<bits>(ratio);
	}

	template<uint32 bits>
	constexpr float32 UnPackToFloat(uint32 ui, float32 min, float32 max)
	{
		assert(min < max);
		return min + UnPackToUnitFloat<bits>(ui) * (max - min);
	}
}
