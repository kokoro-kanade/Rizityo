#pragma once

#pragma warning(disable: 4530) // ��O�̌x���𖳎�

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>

// ���ʃw�b�_�[
#include "PrimitiveTypes.h"
#include "../Utility/Utility.h"
#include "../Utility/MathType.h"

#if defined(_WIN64)
#include <DirectXMath.h>
#endif