#pragma once
#include <thread>

class Simulation
{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0; // TODO : ���邩�ǂ���
	virtual void Shutdown() = 0;
};