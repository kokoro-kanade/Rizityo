#pragma once
#include <thread>

class Simulation
{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0; // TODO : ‚¢‚é‚©‚Ç‚¤‚©
	virtual void Shutdown() = 0;
};