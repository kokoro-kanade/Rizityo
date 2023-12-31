#pragma once
#include <cstdint>

// 符号なし整数
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// 符号付き整数
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

// 最大値を無効な値とみなす
constexpr uint64 UINT64_INVALID_NUM{ 0xffff'ffff'ffff'ffff }; 
constexpr uint32 UINT32_INVALID_NUM{ 0xffff'ffff };
constexpr uint16 UINT16_INVALID_NUM{ 0xffff };
constexpr uint8 UINT8_INVALID_NUM{ 0xff };

using float32 = float;