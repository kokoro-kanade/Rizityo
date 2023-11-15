#pragma once

#pragma warning(disable: 4530) // 例外の警告を無視

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <mutex>

#if defined(_WIN64)
#include <DirectXMath.h>
#endif

#ifdef _DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif // _DEBUG

#ifndef OUT
#define OUT
#endif // OUT

// 共通ヘッダー
#include "PrimitiveTypes.h"
#include "Id.h"
#include "../Utility/Utility.h"
#include "../Utility/MathType.h"
#include "../Utility/Math.h"

#ifndef DISABLE_COPY
#define DISABLE_COPY(T)				  \
explicit T(const T&) = delete;		  \
T& operator=(const T&) = delete;
#endif // DISABLE_COPY

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)				  \
explicit T(const T&&) = delete;		  \
T& operator=(const T&&) = delete;
#endif // DISABLE_MOVE

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif // !DISABLE_COPY_AND_MOVE


