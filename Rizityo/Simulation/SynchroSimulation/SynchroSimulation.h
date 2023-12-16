#pragma once
#include "../Simulation.h"
#include "API/GameEntity.h"

using namespace Rizityo;

class SynchroSimulation : public Simulation
{
public:
	void Initialize() override;
	void Update() override;
	void Shutdown() override;
};

namespace Oscillator
{
	uint32 GetOscillatorNum();
	const GameEntity::Entity* const GetOscillatorEntity();
	const Math::Vector3* const GetOscillatorPositions();
	const float32* const GetOscillatorPhases();
	uint32 GetEntityIndex(ID::IDType id);

	void ApplyWallCondition(OUT Math::Vector3& pos);
}