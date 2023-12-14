#include "Boid.h"
#include "BoidSimulation.h"

REGISTER_SCRIPT(BoidScript);

void BoidScript::BeginPlay() {}

void BoidScript::Update(float32 dt)
{
	using namespace Math;
	
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

		// ãﬂê⁄óÃàÊì‡ÇæÇØÇçló∂
		if (dist > _NeighborRadius)
			continue;

		// éãäEÇ…ì¸Ç¡ÇƒÇ¢ÇÈÇ‡ÇÃÇæÇØÇçló∂
		Vector3 myVec{ _Verocity };
		myVec.Normalize();
		dir.Normalize();
		float32 th = dir.Dot(myVec);
		if (th < cosf(_FOV * PI / 180.f))
			continue;

		neighborCount++;

		// êÆóÒ
		totalNeighborVerocity += verocities[i];

		// åãçá
		totalNeighborPosition += otherPos;

		// ï™ó£ÇÕîrëºóÃàÊì‡ÇæÇØÇçló∂
		if (dist > _SeperationRadius)
			continue;

		seperationCount++;

		// ï™ó£
		totalSeperationPosition += otherPos;
	}

	Vector3 accel{ Vector3::ZERO };

	accel += Boid::CalcWallForce(myPos); // ï«ÇîÇØÇÈóÕ

	if (neighborCount > 0)
	{
		accel += (totalNeighborPosition / neighborCount - myPos) * _Cohesion;		 // åãçá
		accel += (totalNeighborVerocity / neighborCount - _Verocity) * _Alignement;	 // êÆóÒ
	}

	if (seperationCount > 0)
	{
		accel -= (totalSeperationPosition / seperationCount - myPos) * _Seperation;	 // ï™ó£
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
