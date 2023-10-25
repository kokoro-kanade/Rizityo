#pragma once
#include <stdint.h>

// •„†‚È‚µ®”
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// •„†•t‚«®”
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

constexpr uint64 UINT64_INVALID_NUM{ 0xffff'ffff'ffff'ffffui64 }; // Å‘å’l‚ğ–³Œø‚È’l‚Æ‚İ‚È‚·
constexpr uint32 UINT32_INVALID_NUM{ 0xffff'ffffui32 };
constexpr uint16 UINT16_INVALID_NUM{ 0xffffui16 };
constexpr uint8 UINT8_INVALID_NUM{ 0xffui8 };

using float32 = float;