#include "Quaternion.h"

namespace Rizityo::Math
{
#if defined(_WIN64)

	const Quaternion Quaternion::IDENTITY(0.0f, 0.0f, 0.0f, 1.0f);


	Vector3 Quaternion::Transform(const Vector3& vector) const
	{
		DirectX::XMVECTOR self = *this;
		return Vector3(DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(self, vector), DirectX::XMQuaternionConjugate(self)));
	}

	Quaternion Quaternion::FromToRotation(const Math::Vector3& from, const Math::Vector3& to)
	{
		using namespace DirectX;
		XMVECTOR fromNormalized = XMVector3Normalize(from);
		XMVECTOR toNormalized = XMVector3Normalize(to);

		float dot = XMVectorGetX(XMVector3Dot(fromNormalized, toNormalized));

		if (dot >= 1.f) // “¯‚¶Œü‚«
		{
			return IDENTITY; // •Ï‰»‚È‚µ
		}
		else if (dot <= -1.f) // ³”½‘Î‚ðŒü‚¢‚Ä‚¢‚é
		{
			XMVECTOR axis = XMVector3Cross(fromNormalized, Vector3::RIGHT); // from‚Æright‚ÌŠOÏ‚ð‚Æ‚Á‚Ä‰ñ“]Ž²‚ð‹‚ß‚é
			if (XMVector3NearEqual(XMVector3LengthSq(axis), g_XMZero, g_XMEpsilon)) // from‚ªright‚Æ“¯‚¶Œü‚«‚È‚çup‚ÆŠOÏ‚ð‚Æ‚é
			{
				axis = XMVector3Cross(fromNormalized, Vector3::UP);
			}

			const XMVECTOR rot = XMQuaternionRotationAxis(axis, Math::PI);
			return Quaternion{ rot };
		}
		else
		{
			const XMVECTOR cross = XMVector3Cross(fromNormalized, toNormalized);
			Quaternion rot{ cross };

			const float s = sqrtf((1.f + dot) * 2.f);
			// ³‹K‰»
			rot.x /= s;
			rot.y /= s;
			rot.z /= s;
			rot.w = s * 0.5f; // cos(theta/2)

			return rot;
		}
	}

	Quaternion Quaternion::LookRotation(const Math::Vector3& look, const Math::Vector3& up/* = Math::Vector3::UP*/)
	{
		using namespace DirectX;

		Quaternion q1 = FromToRotation(Vector3::FORWARD, look);

		const XMVECTOR cross = XMVector3Cross(look, up);
		if (XMVector3NearEqual(XMVector3LengthSq(cross), g_XMZero, g_XMEpsilon))
		{
			return q1;
		}

		const XMVECTOR u = XMQuaternionMultiply(q1, Vector3::UP);

		Quaternion q2 = FromToRotation(Vector3{ u }, up);

		return Quaternion{ XMQuaternionMultiply(q2, q1) };
	}

#endif // defined(_WIN64)

}