#pragma once
#include "../Simulation.h"
#include "API/GameEntity.h"

using namespace Rizityo;

class BoidSimulation : public Simulation
{
public:

	void Initialize() override;
	void Update() override;
	void Shutdown() override;
};

namespace Boid
{
	uint32 GetBoidNum();
	const GameEntity::Entity* const GetBoidEntity();
	const Math::Vector3* const GetBoidPositions();
	const Math::Vector3* const GetBoidVerocities();
	uint32 GetEntityIndex(ID::IDType id);
	
	Math::Vector3 CalcWallForce(Math::Vector3 pos);
}