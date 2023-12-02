#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Math
{
#if defined(_WIN64)

	// XMVECTORとのロードとストアを繰り返し行っているのでオーバーヘッドは大きいが分かりやすさを優先する
	// XMVECTORとのロードとストアの高速化のためにXMFLOAT4Aを利用
	class Vector4 : public DirectX::XMFLOAT4A
	{
	public:

		Vector4() : DirectX::XMFLOAT4A()
		{}

		Vector4(float32 x, float32 y, float32 z, float32 w) : DirectX::XMFLOAT4A(x, y, z, w)
		{}

		explicit Vector4(DX_Vector4 v) : DirectX::XMFLOAT4A(v.x, v.y, v.z, v.w)
		{}

		explicit Vector4(const float32* v) : DirectX::XMFLOAT4A(v)
		{}

		explicit Vector4(DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat4A(this, v);
		}

		Vector4(const Vector4& v) : DirectX::XMFLOAT4A(v)
		{}

		Vector4& operator = (DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat4A(this, v);
			return *this;
		}

		Vector4& operator = (const Vector4& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat4A(this);
		}

		bool operator == (const Vector4& v) const
		{
			return x == v.x && y == v.y && z == v.z && w == v.w;
		}

		bool operator != (const Vector4& v) const
		{
			return x != v.x || y != v.y || z != v.z || w != v.w;
		}

		bool operator < (const Vector4& v) const
		{
			return x < v.x && y < v.y && z < v.z && w < v.w;
		}

		bool operator <= (const Vector4& v) const
		{
			return x <= v.x && y <= v.y && z <= v.z && w <= v.w;
		}

		bool operator > (const Vector4& v) const
		{
			return x > v.x && y > v.y && z > v.z && w > v.w;
		}

		bool operator >= (const Vector4& v) const
		{
			return x >= v.x && y >= v.y && z >= v.z && w >= v.w;
		}

		[[nodiscard]] float32 Length() const;

		[[nodiscard]] float32 LengthSquared() const;

		void Normalise();

		[[nodiscard]] Vector4 Normalise() const;

		[[nodiscard]] float32 Dot(const Vector4& v) const;

		[[nodiscard]] Vector4 Cross(const Vector4& a, const Vector4& b) const;

		Vector4 operator + (const Vector4& v) const
		{
			return Vector4(DirectX::XMVectorAdd(*this, v));
		}

		Vector4 operator - (const Vector4& v) const
		{
			return Vector4(DirectX::XMVectorSubtract(*this, v));
		}

		Vector4 operator * (const Vector4& v) const
		{
			return Vector4(DirectX::XMVectorMultiply(*this, v));
		}

		Vector4 operator / (const Vector4& v) const
		{
			return Vector4(DirectX::XMVectorDivide(*this, v));
		}

		Vector4 operator - () const
		{
			return Vector4(-x, -y, -z, -w);
		}

		Vector4 operator * (const float32 n) const
		{
			return Vector4(DirectX::XMVectorMultiply(*this, DirectX::XMVectorReplicate(n)));
		}

		friend Vector4 operator * (const float32 n, const Vector4& v)
		{
			return Vector4(DirectX::XMVectorMultiply(DirectX::XMVectorReplicate(n), v));
		}

		Vector4 operator / (const float32 n) const
		{
			return Vector4(DirectX::XMVectorDivide(*this, DirectX::XMVectorReplicate(n)));
		}

		Vector4& operator += (const Vector4& v)
		{
			return *this = DirectX::XMVectorAdd(*this, v);
		}

		Vector4& operator -= (const Vector4& v)
		{
			return *this = DirectX::XMVectorSubtract(*this, v);
		}

		Vector4& operator *= (const Vector4& v)
		{
			return *this = DirectX::XMVectorMultiply(*this, v);
		}

		Vector4& operator /= (const Vector4& v)
		{
			return *this = DirectX::XMVectorDivide(*this, v);
		}

		Vector4& operator *= (const float32 n)
		{
			return *this = DirectX::XMVectorMultiply(*this, DirectX::XMVectorReplicate(n));
		}

		Vector4& operator /= (const float32 n)
		{
			return *this = DirectX::XMVectorDivide(*this, DirectX::XMVectorReplicate(n));
		}

		[[nodiscard]] static Vector4 Lerp(const Vector4& a, const Vector4& b, float32 t);

		[[nodiscard]] static Vector4 Min(const Vector4& a, const Vector4& b);

		[[nodiscard]] static Vector4 Max(const Vector4& a, const Vector4& b);

		static const Vector4 ZERO;
		static const Vector4 UNIT_X;
		static const Vector4 UNIT_Y;
		static const Vector4 UNIT_Z;
		static const Vector4 UNIT_W;
	};


#endif // defined(_WIN64)
}