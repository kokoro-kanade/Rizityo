#pragma once
#include "API/GameEntity.h"
#include <random>

using namespace Rizityo;

class BoidScript : public Script::EntityScript
{
public:

	explicit BoidScript(GameEntity::Entity entity) : Script::EntityScript(entity)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());

		// ˆê—lÀ”•ª•z
		static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

		Math::Vector3 dir = { dist(gen), 0.f, dist(gen) };
		dir.Normalize();
		_Verocity = dir * dist(gen);

	}

	void BeginPlay() override;

	void Update(float32 dt) override;

	[[nodiscard]] Math::Vector3 GetVerocity() { return _Verocity; }

private:

	Math::Vector3 _Verocity = { 1.f, 0.f, 0.f };
	constexpr static float32 _MinSpeed = 2.f;
	constexpr static float32 _MaxSpeed = 4.f;

	float32 _NeighborRadius = 4.f; // ‹ßÚ—Ìˆæ‚Ì”¼Œa(m)
	float32 _SeperationRadius = 3.f; // ”r‘¼—Ìˆæ‚Ì”¼Œa(m)
	float32 _FOV = 20.f; // ‹ŠE‚Ì”¼•ª‚ÌŠp“x(“x)

	float32 _Alignement = 1.f;
	float32 _Cohesion = 1.f;
	float32 _Seperation = 1.f;
};