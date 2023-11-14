#include "CommonHeaders.h"
#include <filesystem>

#ifdef _WIN64

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <crtdbg.h>

namespace
{
	std::filesystem::path SetCurrentDirectoryToExectutablePath()
	{
		// ワーキングディレクトリを実行ファイルのパスに変更
		wchar_t path[MAX_PATH]{};
		const uint32 length = GetModuleFileName(0, &path[0], MAX_PATH);
		if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			return {};
		std::filesystem::path p{ path };
		std::filesystem::current_path(p.parent_path());
		return std::filesystem::current_path();
	}
}

#ifndef USE_EDITOR

extern bool EngineInitialize();
extern void EngineUpdate();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SetCurrentDirectoryToExectutablePath();

	if (EngineInitialize())
	{
		MSG msg{};
		bool isRunning = true;
		while (isRunning)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				isRunning &= (msg.message != WM_QUIT);
			}

			EngineUpdate();
		}
	}

	EngineShutdown();
	return 0;
}

#endif // !USE_EDITOR

#endif // _WIN64