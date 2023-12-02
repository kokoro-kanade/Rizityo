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

#endif // defined(_WIN64)

}