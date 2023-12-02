#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Math
{
	constexpr float32 PI = 3.1415926535897932384626433832795;
	constexpr float32 TWO_PI = 2.f * PI;
	constexpr float32 HALF_PI = 0.5f * PI;
	constexpr float32 EPSILON = 1e-5f;

#if defined(_WIN64)
	using DX_Vector2 = DirectX::XMFLOAT2;
	using DX_Vector2a = DirectX::XMFLOAT2A;
	using DX_Vector3 = DirectX::XMFLOAT3;
	using DX_Vector3a = DirectX::XMFLOAT3A;
	using DX_Vector4 = DirectX::XMFLOAT4;
	using DX_Vector4a = DirectX::XMFLOAT4A;
	using DX_U32Vector2 = DirectX::XMUINT2;
	using DX_U32Vector3 = DirectX::XMUINT3;
	using DX_U32Vector4 = DirectX::XMUINT4;
	using DX_I32Vector2 = DirectX::XMINT2;
	using DX_I32DX_Vector3 = DirectX::XMINT3;
	using DX_I32DX_Vector4 = DirectX::XMINT4;
	using DX_Matrix3x3 = DirectX::XMFLOAT3X3;
	using DX_Matrix4x4 = DirectX::XMFLOAT4X4;
	using DX_Matrix4x4a = DirectX::XMFLOAT4X4A;
#endif

}