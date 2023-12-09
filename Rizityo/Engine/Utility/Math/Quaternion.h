#pragma once
#include "../Common/CommonHeaders.h"
#include "../Utility/Math/Vector3.h"


namespace Rizityo::Math
{
#if defined(_WIN64)

	// XMVECTORとのロードとストアを繰り返し行っているのでオーバーヘッドは大きいが分かりやすさを優先する
	// XMVECTORとのロードとストアの高速化のためにXMFLOAT4Aを利用
	class Quaternion : public DirectX::XMFLOAT4A
	{
	public:

		Quaternion()
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionIdentity());
		}

		constexpr Quaternion(float32 x, float32 y, float32 z, float32 w) : DirectX::XMFLOAT4A(x, y, z, w)
		{}

		explicit Quaternion(DX_Vector4 v) : DirectX::XMFLOAT4A(v.x, v.y, v.z, v.w)
		{}

		Quaternion(float32 pitch, float32 yaw, float32 roll)
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
		}

		Quaternion(const Vector3& rotation)
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionRotationRollPitchYawFromVector(rotation));
		}

		Quaternion(const Vector3& axis, float32 angle)
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionRotationAxis(axis, angle));
		}

		explicit Quaternion(const float32* elements) : DirectX::XMFLOAT4A(elements)
		{}

		explicit Quaternion(DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat4A(this, v);
		}

		Quaternion(const Quaternion& q) : DirectX::XMFLOAT4A(q)
		{}

		Quaternion& operator = (DirectX::FXMVECTOR v)
		{
			DirectX::XMStoreFloat4A(this, v);
			return *this;
		}

		Quaternion& operator = (const Quaternion& q)
		{
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
			return *this;
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat4A(this);
		}

		bool operator == (const Quaternion& q) const
		{
			return x == q.x && y == q.y && z == q.z && w == q.w;
		}

		bool operator != (const Quaternion& q) const
		{
			return x != q.x || y != q.y || z != q.z || w != q.w;
		}

		[[nodiscard]] bool IsIdentity() const
		{
			return DirectX::XMQuaternionIsIdentity(*this);
		}

		void FromAxisAngle(const Vector3& axis, float32 angle)
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionRotationAxis(axis, angle));
		}

		void ToAxisAngle(Vector3& axis, float32& angle) const
		{
			DirectX::XMVECTOR a;
			DirectX::XMQuaternionToAxisAngle(&a, &angle, *this);
			axis = a;
		}

		[[nodiscard]] float32 Dot(const Quaternion& q) const
		{
			return DirectX::XMVectorGetX(DirectX::XMQuaternionDot(*this, q));
		}

		[[nodiscard]] float32 Length() const
		{
			return DirectX::XMVectorGetX(DirectX::XMQuaternionLength(*this));
		}

		[[nodiscard]] float32 LengthSquared() const
		{
			return DirectX::XMVectorGetX(DirectX::XMQuaternionLengthSq(*this));
		}

		void Normalise()
		{
			DirectX::XMStoreFloat4A(this, DirectX::XMQuaternionNormalize(*this));
		}

		[[nodiscard]] Quaternion Conjugate() const
		{
			return Quaternion(DirectX::XMQuaternionConjugate(*this));
		}

		[[nodiscard]] Quaternion Inverse() const
		{
			return Quaternion(DirectX::XMQuaternionInverse(*this));
		}

		[[nodiscard]] Quaternion Exp() const
		{
			return Quaternion(DirectX::XMQuaternionExp(*this));
		}

		[[nodiscard]] Quaternion Ln() const
		{
			return Quaternion(DirectX::XMQuaternionLn(*this));
		}

		[[nodiscard]] Quaternion Slerp(const Quaternion& q, float32 t) const
		{
			return Quaternion(DirectX::XMQuaternionSlerp(*this, q, t));
		}

		Vector3 Transform(const Vector3& v) const;

		static Quaternion FromToRotation(const Math::Vector3& from, const Math::Vector3& to);

		static Quaternion LookRotation(const Math::Vector3& forward, const Math::Vector3& up = Math::Vector3::UP);

		static const Quaternion IDENTITY;
	};

#endif // defined(_WIN64)

}