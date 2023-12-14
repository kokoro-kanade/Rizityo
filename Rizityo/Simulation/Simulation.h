#pragma once
#include <thread>

class Simulation
{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0; // TODO : いるかどうか
	virtual void Shutdown() = 0;
};