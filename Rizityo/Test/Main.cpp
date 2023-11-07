#pragma comment(lib, "Engine.lib")


#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#else
#error �ǂꂩ��e�X�g��L���ɂ��Ă�������
#endif

#ifdef _WIN64

#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	EngineTest test{};
	if (test.Initialize())
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

			test.Run();
		}
	}

	test.Shutdown();
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