#pragma once
#include "../Common/CommonHeaders.h"

namespace Rizityo::Math
{
#if defined(_WIN64)

	// XMVECTORとのロードとストアを繰り返し行っているのでオーバーヘッドは大きいが分かりやすさを優先する
	// XMVECTORとのロードとストアの高速化のためにXMFLOAT3Aを利用
	class Vector3 : public DirectX::XMFLOAT3A
	{
	public:

		constexpr Vector3() : DirectX::XMFLOAT3A()
		{}

		constexpr Vector3(float32 x, float32 y, float32 z) : DirectX::XMFLOAT3A(x, y, z)
		{}

		explicit Vector3(DX_Vector3 v) : DirectX::XMFLOAT3A(v.x, v.y, v.z)
		{}

		explicit Vector3(const float32* v) : DirectX::XMFLOAT3A(v)
		{}

		explicit Vector3(DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat3A(this, v);
		}

		Vector3(const Vector3& v) : DirectX::XMFLOAT3A(v)
		{}

		Vector3& operator = (DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat3A(this, v);
			return *this;
		}

		Vector3& operator = (const Vector3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat3A(this);
		}

		bool operator == (const Vector3& v) const
		{
			return x == v.x && y == v.y && z == v.z;
		}

		bool operator != (const Vector3& v) const
		{
			return x != v.x || y != v.y || z != v.z;
		}

		bool operator < (const Vector3& v) const
		{
			return x < v.x && y < v.y && z < v.z;
		}

		bool operator <= (const Vector3& v) const
		{
			return x <= v.x && y <= v.y && z <= v.z;
		}

		bool operator > (const Vector3& v) const
		{
			return x > v.x && y > v.y && z > v.z;
		}

		bool operator >= (const Vector3& v) const
		{
			return x >= v.x && y >= v.y && z >= v.z;
		}

		[[nodiscard]] float32 Length() const;

		[[nodiscard]] float32 LengthSquared() const;

		void Normalize();

		[[nodiscard]] Vector3 Normalize() const;

		[[nodiscard]] float32 Dot(const Vector3& v) const;

		[[nodiscard]] Vector3 Cross(const Vector3& v) const;

		Vector3 operator + (const Vector3& v) const
		{
			return Vector3(DirectX::XMVectorAdd(*this, v));
		}

		Vector3 operator - (const Vector3& v) const
		{
			return Vector3(DirectX::XMVectorSubtract(*this, v));
		}

		Vector3 operator * (const Vector3& v) const
		{
			return Vector3(DirectX::XMVectorMultiply(*this, v));
		}

		Vector3 operator / (const Vector3& v) const
		{
			return Vector3(DirectX::XMVectorDivide(*this, v));
		}

		Vector3 operator - () const
		{
			return Vector3(-x, -y, -z);
		}

		Vector3 operator * (const float32 n) const
		{
			return Vector3(DirectX::XMVectorMultiply(*this, DirectX::XMVectorReplicate(n)));
		}

		friend Vector3 operator * (const float32 n, const Vector3& v)
		{
			return Vector3(DirectX::XMVectorMultiply(DirectX::XMVectorReplicate(n), v));
		}

		Vector3 operator / (const float32 n) const
		{
			return Vector3(DirectX::XMVectorDivide(*this, DirectX::XMVectorReplicate(n)));
		}

		Vector3& operator += (const Vector3& v)
		{
			return *this = DirectX::XMVectorAdd(*this, v);
		}

		Vector3& operator -= (const Vector3& v)
		{
			return *this = DirectX::XMVectorSubtract(*this, v);
		}

		Vector3& operator *= (const Vector3& v)
		{
			return *this = DirectX::XMVectorMultiply(*this, v);
		}

		Vector3& operator /= (const Vector3& v)
		{
			return *this = DirectX::XMVectorDivide(*this, v);
		}

		Vector3& operator *= (const float32 n)
		{
			return *this = DirectX::XMVectorMultiply(*this, DirectX::XMVectorReplicate(n));
		}

		Vector3& operator /= (const float32 n)
		{
			return *this = DirectX::XMVectorDivide(*this, DirectX::XMVectorReplicate(n));
		}

		[[nodiscard]] static Vector3 Lerp(const Vector3& a, const Vector3& b, float32 t);

		[[nodiscard]] static Vector3 Min(const Vector3& a, const Vector3& b);

		[[nodiscard]] static Vector3 Max(const Vector3& a, const Vector3& b);

		static const Vector3 ZERO;
		static const Vector3 UNIT_X;
		static const Vector3 UNIT_Y;
		static const Vector3 UNIT_Z;
		static const Vector3 UP;
		static const Vector3 DOWN;
		static const Vector3 RIGHT;
		static const Vector3 LEFT;
		static const Vector3 FORWARD;
		static const Vector3 BACKWORD;
	};

#endif // defined(_WIN64)
}
