#include "Common.h"
#include "CommonHeaders.h"
#include "../Engine/Components/Script.h"
#include "../Graphics/Renderer.h"
#include "../Platform/PlatformTypes.h"
#include "../Platform/Platform.h"

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

	Utility::Vector<Graphics::RenderSurface> Surfaces;
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

EDITOR_INTERFACE
uint32 CreateRenderSurface(HWND host, int32 width, int32 height)
{
	assert(host);
	Platform::WindowInitInfo info{ nullptr, host, nullptr, 0, 0, width, height };
	Graphics::RenderSurface surface{ Platform::Create_Window(&info), {} };
	assert(surface.Window.IsValid());
	Surfaces.emplace_back(surface);
	return (uint32)Surfaces.size() - 1;
}

EDITOR_INTERFACE
void RemoveRenderSurface(uint32 id)
{
	assert(id < Surfaces.size());
	Platform::Remove_Window(Surfaces[id].Window.GetId());
}

EDITOR_INTERFACE
HWND GetWindowHandle(uint32 id)
{
	assert(id < Surfaces.size());
	return (HWND)Surfaces[id].Window.GetHandle();
}

EDITOR_INTERFACE
void ResizeRenderSurface(uint32 id)
{
	assert(id < Surfaces.size());
	Surfaces[id].Window.Resize(0, 0);
}