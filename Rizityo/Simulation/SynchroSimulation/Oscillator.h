#pragma once
#include "API/GameEntity.h"
#include <random>

using namespace Rizityo;

class OscillatorScript : public Script::EntityScript
{
public:

	explicit OscillatorScript(GameEntity::Entity entity) : Script::EntityScript(entity)
	{
		static std::uniform_real_distribution<float32> dist(0.f, Math::TWO_PI);

		_DirAngle = dist(_Gen);
		_AngularFreq = dist(_Gen) / 4;
		_Phase = dist(_Gen);
	}

	void BeginPlay() override;

	void Update(float32 dt) override;

	[[nodiscard]] constexpr float32 GetPhase() const { return _Phase; }

private:

	float32 _Speed = 2.f;
	float32 _DirAngle = 0.f;

	float32 _Phase = 0.f;
	float32 _AngularFreq = Math::HALF_PI / 2;
	float32 _NeighborRadius = 4.f; // ãﬂê⁄óÃàÊÇÃîºåa(m)
	float32 _Weight = 1.f;

	static inline std::random_device _RD{};
	static inline std::mt19937 _Gen{_RD()};

};