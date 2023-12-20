#include "Boid.h"
#include "BoidSimulation.h"

REGISTER_SCRIPT(BoidScript);

void BoidScript::BeginPlay() {}

void BoidScript::Update(float32 dt)
{
	using namespace Math;

	if (!Boid::GetUpdateFlag())
		return;

	float32 alignementWeight = Boid::GetAlignement();
	float32 cohesionWeight = Boid::GetCohesion();
	float32 seperationWeight = Boid::GetSeperation();
	float32 neighborRadius = Boid::GetNeighborRadius();
	float32 seperationRadius = Boid::GetSeperationRadius();
	float32 fOV = Boid::GetFOV();

	Vector3 totalNeighborVerocity{ Vector3::ZERO };
	Vector3 totalNeighborPosition{ Vector3::ZERO };
	Vector3 totalSeperationPosition{ Vector3::ZERO };
	uint32 neighborCount = 0;
	uint32 seperationCount = 0;

	const uint32 num = Boid::GetBoidNum();
	const GameEntity::Entity* const entities{ Boid::GetBoidEntity() };
	const Vector3* const positions{ Boid::GetBoidPositions() };
	const Vector3* const verocities{ Boid::GetBoidVerocities() };

	const uint32 index = Boid::GetEntityIndex(ID());
	assert(index < num);
	Vector3 myPos{ positions[index] };
	for (uint32 i = 0; i < num; i++)
	{
		if (i == index)
			continue;

		Vector3 otherPos{ positions[i] };
		Vector3 dir{ otherPos - myPos };
		float32 dist = dir.Length();

		// �ߐڗ̈���������l��
		if (dist > neighborRadius)
			continue;

		// ���E�ɓ����Ă�����̂������l��
		Vector3 myVec{ _Verocity };
		myVec.Normalize();
		dir.Normalize();
		float32 th = dir.Dot(myVec);
		if (th < cosf(fOV * PI / 180.f))
			continue;

		neighborCount++;

		// ����
		totalNeighborVerocity += verocities[i];

		// ����
		totalNeighborPosition += otherPos;

		// �����͔r���̈���������l��
		if (dist > seperationRadius)
			continue;

		seperationCount++;

		// ����
		totalSeperationPosition += otherPos;
	}

	Vector3 accel{ Vector3::ZERO };

	accel += Boid::CalcWallForce(myPos); // �ǂ�������

	if (neighborCount > 0)
	{
		accel += (totalNeighborPosition / neighborCount - myPos) * cohesionWeight;		 // ����
		accel += (totalNeighborVerocity / neighborCount - _Verocity) * alignementWeight;	 // ����
	}

	if (seperationCount > 0)
	{
		accel -= (totalSeperationPosition / seperationCount - myPos) * seperationWeight;	 // ����
	}

	_Verocity += accel * dt;
	float32 speed = Clamp(_Verocity.Length(), _MinSpeed, _MaxSpeed);
	_Verocity.Normalize();
	_Verocity *= speed;

	myPos += _Verocity * dt;
	SetPosition(myPos);

	Quaternion rot{ Quaternion::LookRotation(_Verocity) };
	SetRotation(rot);
}
