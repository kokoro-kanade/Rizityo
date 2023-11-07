#pragma once

#pragma warning(disable: 4530) // 例外の警告を無視

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>

#if defined(_WIN64)
#include <DirectXMath.h>
#endif

// 共通ヘッダー
#include "PrimitiveTypes.h"
#include "Id.h"
#include "..\Utility\Utility.h"
#include "..\Utility\MathType.h"

#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x) (void(0))
#endif // _DEBUG

