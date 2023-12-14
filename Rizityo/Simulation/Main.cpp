#include "Simulation.h"

#pragma comment(lib, "Engine.lib")

#include "BoidSimulation/BoidSimulation.h"

#ifdef _WIN64

#include <Windows.h>
#include <filesystem>

namespace
{
	std::filesystem::path SetCurrentDirectoryToExectutablePath()
	{
		// ワーキングディレクトリを実行ファイルのパスに変更
		wchar_t path[MAX_PATH]{};
		const uint32_t length = GetModuleFileName(0, &path[0], MAX_PATH);
		if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			return {};
		std::filesystem::path p{ path };
		std::filesystem::current_path(p.parent_path());
		return std::filesystem::current_path();
	}
}

extern bool Initialize();
extern void Update();
extern void Shutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SetCurrentDirectoryToExectutablePath();

	if (Initialize())
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

			Update();
		}
	}

	Shutdown();
	return 0;
}

#else

int main()
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	EngineTest test;

	if (test.Initialize())
	{
		test.Run();
	}

	test.Shutdown();
}

#endif // _WIN64