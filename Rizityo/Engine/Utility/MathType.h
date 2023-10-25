#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Math
{
	constexpr float PI = 3.1415926535897932384626433832795;
	constexpr float EPSILON = 1e-5f;

#if defined(_WIN64)
	using Vector2 = DirectX::XMFLOAT2;
	using Vector2a = DirectX::XMFLOAT2A;
	using Vector3 = DirectX::XMFLOAT3;
	using Vector3a = DirectX::XMFLOAT3A;
	using Vector4 = DirectX::XMFLOAT4;
	using Vector4a = DirectX::XMFLOAT4A;
	using U32Vector2 = DirectX::XMUINT2;
	using U32Vector3 = DirectX::XMUINT3;
	using U32Vector4 = DirectX::XMUINT4;
	using I32Vector2 = DirectX::XMINT2;
	using I32Vector3 = DirectX::XMINT3;
	using I32Vector4 = DirectX::XMINT4;
	using Matrix3x3 = DirectX::XMFLOAT3X3;
	using Matrix4x4 = DirectX::XMFLOAT4X4;
	using Matrix4x4a = DirectX::XMFLOAT4X4A;
#endif
}