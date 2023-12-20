#pragma once
#include "../Simulation.h"
#include "API/GameEntity.h"
#include "API/GUI.h"

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

	float32 GetSpeed();
	float32 GetNeighborRadius();
	float32 GetWeight();
	bool GetUpdateFlag();
}

class SynchroSimulationGUI : public GUI::UI_Base
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