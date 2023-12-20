#pragma once
#include "../Simulation.h"
#include "API/GameEntity.h"
#include "API/GUI.h"

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

	float32 GetAlignement();
	float32 GetCohesion();
	float32 GetSeperation();
	float32 GetNeighborRadius();
	float32 GetSeperationRadius();
	float32 GetFOV();
	bool GetUpdateFlag();

}

class BoidSimulationGUI : public GUI::UI_Base
{
public:

	using UI_Base::UI_Base;
	void ShowContent() override;
	void Update(float32 dt) override;

private:
	float32 _CoolTime = 0.1f;
	float32 _ElapsedTime = 0.f;
	bool _IsCooling = false;
};