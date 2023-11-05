#include "Common.h"
#include "CommonHeaders.h"
#include "../Engine/Components/Script.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <atlsafe.h>

using namespace Rizityo;

namespace
{
	HMODULE GameCodeDll{ nullptr };
	using _GetScriptCreateFunc = Rizityo::Script::Internal::ScriptCreateFunc(*)(size_t);
	_GetScriptCreateFunc GetScriptCreateFunc{ nullptr };
	using _GetScriptNames = LPSAFEARRAY(*)(void);
	_GetScriptNames GetScriptNames{ nullptr };
}

EDITOR_INTERFACE
uint32 LoadGameCodeDll(const char* dllPath)
{
	if (GameCodeDll)
		return FALSE;
	GameCodeDll = LoadLibraryA(dllPath);
	assert(GameCodeDll);

	GetScriptCreateFunc = (_GetScriptCreateFunc)GetProcAddress(GameCodeDll, "GetScriptCreateFunc");
	GetScriptNames = (_GetScriptNames)GetProcAddress(GameCodeDll, "GetScriptNames");

	return (GameCodeDll && GetScriptCreateFunc && GetScriptNames) ? TRUE : FALSE;
}

EDITOR_INTERFACE
uint32 UnLoadGameCodeDll()
{
	if (!GameCodeDll)
		return FALSE;
	assert(GameCodeDll);
	int result = FreeLibrary(GameCodeDll);
	GameCodeDll = nullptr;
	return TRUE;
}

EDITOR_INTERFACE
Script::Internal::ScriptCreateFunc GetGameScriptCreateFunc(const char* name)
{
	return (GameCodeDll && GetScriptCreateFunc) ? GetScriptCreateFunc(Script::Internal::StringHash()(name)) : nullptr;
}

EDITOR_INTERFACE
LPSAFEARRAY GetGameScriptNames()
{
	return (GameCodeDll && GetScriptNames) ? GetScriptNames() : nullptr;
}