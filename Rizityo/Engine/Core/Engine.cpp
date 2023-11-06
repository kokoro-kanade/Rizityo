#if !defined(SHIPPING)

#include "../Content/ContentLoader.h"
#include "../Components/Script.h"
#include <thread>

bool EngineInitialize()
{
	bool result = Rizityo::Content::LoadGame();
	return result;
}

void EngineUpdate()
{
	Rizityo::Script::Update(10.f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void EngineShutdown()
{
	Rizityo::Content::UnLoadGame();
}

#endif