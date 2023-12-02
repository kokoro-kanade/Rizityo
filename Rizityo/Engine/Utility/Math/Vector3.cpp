#include "Vector3.h"

namespace Rizityo::Math
{
#if defined(_WIN64)

	const Vector3 Vector3::ZERO(0.0f, 0.0f, 0.0f);
	const Vector3 Vector3::UNIT_X(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::UNIT_Y(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::UNIT_Z(0.0f, 0.0f, 1.0f);


	float Vector3::Length() const
	{
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(*this));
	}

	float Vector3::LengthSquared() const
	{
		return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(*this));
	}

	void Vector3::Normalise()
	{
		*this = DirectX::XMVector3Normalize(*this);
	}

	Vector3 Vector3::Normalise() const
	{
		return Vector3(DirectX::XMVector3Normalize(*this));
	}

	float Vector3::Dot(const Vector3& v) const
	{
		return DirectX::XMVectorGetX(DirectX::XMVector3Dot(*this, v));
	}

	Vector3 Vector3::Cross(const Vector3& v) const
	{
		return Vector3(DirectX::XMVector3Cross(*this, v));
	}

	Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t)
	{
		return Vector3(DirectX::XMVectorLerp(a, b, t));
	}

	Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
	{
		return Vector3(DirectX::XMVectorMin(a, b));
	}

	Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
	{
		return Vector3(DirectX::XMVectorMax(a, b));
	}

#endif // defined(_WIN64)
}