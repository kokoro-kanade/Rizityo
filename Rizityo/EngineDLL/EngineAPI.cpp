#include "Common.h"
#include "CommonHeaders.h"

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

using namespace Rizityo;

namespace
{
	HMODULE gameCodeDll{ nullptr };
}

EDITOR_INTERFACE
uint32 LoadGameCodeDll(const char* dllPath)
{
	if (gameCodeDll)
		return FALSE;
	LoadLibraryA(dllPath);
	assert(gameCodeDll);

	return gameCodeDll ? TRUE : FALSE;
}

EDITOR_INTERFACE
uint32 UnLoadGameCodeDll()
{
	if (!gameCodeDll)
		return FALSE;
	assert(gameCodeDll);
	int result = FreeLibrary(gameCodeDll);
	gameCodeDll = nullptr;
	return TRUE;
}