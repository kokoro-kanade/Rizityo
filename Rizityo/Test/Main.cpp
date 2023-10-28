#pragma comment(lib, "Engine.lib")


#define TEST_ENTITY_COMPONENTS 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#else
#error どれか一つテストを有効にしてください
#endif

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