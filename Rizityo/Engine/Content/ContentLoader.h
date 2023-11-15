#pragma once
#include "CommonHeaders.h"


#if !defined(SHIPPING)
namespace Rizityo::Content
{
	bool LoadGame();
	void UnLoadGame();

	bool LoadEngineShaders(OUT std::unique_ptr<uint8[]>& shaders, OUT uint64& size);
}
#endif // SHIPPING
