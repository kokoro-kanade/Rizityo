#include "Oscillator.h"
#include "SynchroSimulation.h"

REGISTER_SCRIPT(OscillatorScript);

void OscillatorScript::BeginPlay() {}

void OscillatorScript::Update(float32 dt)
{
	using namespace Math;

	static std::uniform_real_distribution<float32> angleDistribution(-Math::TWO_PI, Math::TWO_PI);

	if (!Oscillator::GetUpdateFlag())
		return;

	float32 Speed = Oscillator::GetSpeed();
	float32 NeighborRadius = Oscillator::GetNeighborRadius();
	float32 Weight = Oscillator::GetWeight();

	Vector3 myPos{ GetPosition() };

	// 乱数を用いて方向を更新して位置更新
	_DirAngle += angleDistribution(_Gen) * dt;
	if (_DirAngle > Math::TWO_PI)
	{
		_DirAngle -= Math::TWO_PI;
	}
	else if (_DirAngle < 0)
	{
		_DirAngle += Math::TWO_PI;
	}
	Vector3 verocity{ Speed * cosf(_DirAngle), 0.f, Speed * sinf(_DirAngle) };
	Vector3 newPos{ myPos + verocity * dt };
	// 周期的境界条件
	Oscillator::ApplyWallCondition(newPos);
	SetPosition(newPos);

	// ほかのオシレーターの状態から位相を更新
	const uint32 num = Oscillator::GetOscillatorNum();
	const GameEntity::Entity* const entities{ Oscillator::GetOscillatorEntity() };
	const Vector3* const positions{ Oscillator::GetOscillatorPositions() };
	const float32* const phases{ Oscillator::GetOscillatorPhases() };

	const uint32 index = Oscillator::GetEntityIndex(ID());
	assert(index < num);
	float32 total = 0;
	for (uint32 i = 0; i < num; i++)
	{
		if (i == index)
			continue;

		Vector3 otherPos{ positions[i] };
		float32 distSquare = (otherPos - myPos).LengthSquared();
		if (distSquare > NeighborRadius * NeighborRadius)
			continue;

		// TODO : 対ごとに重みをつけるかどうか(ex. 距離で減衰する重み)
		total += sinf(phases[i] - _Phase) * Weight;
	}

	_Phase += (_AngularFreq + total) * dt;
	if (_Phase > Math::TWO_PI)
	{
		_Phase -= Math::TWO_PI;
	}
	else if (_Phase < 0)
	{
		_Phase += Math::TWO_PI;
	}
	Quaternion rot{ 0.f, 0.f, _Phase };
	SetRotation(rot);
}