#pragma once

#pragma warning(disable: 4530) // 例外の警告を無視

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>

// 共通ヘッダー
#include "PrimitiveTypes.h"
#include "../Utility/Utility.h"
#include "../Utility/MathType.h"

#if defined(_WIN64)
#include <DirectXMath.h>
#endif