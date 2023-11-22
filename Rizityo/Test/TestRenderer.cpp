#include "TestRenderer.h"
#include "../Platform/PlatformTypes.h"
#include "../Platform/Platform.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Direct3D12/D3D12Core.h"
#include "../Content/ContentToEngine.h"
#include "ShaderCompile.h"
#include <filesystem>
#include <fstream>

#if TEST_RENDERER

using namespace Rizityo;

#define ENABLE_TEST_WORKERS 1

constexpr uint32 NumThreads = 4;
bool Shutdown = false;
std::thread Workers[NumThreads];

Utility::Vector<uint8> Buffer(1024 * 1024, 0);

// uploadのテスト用ワーカー
void BufferTestWorker()
{
	while (!Shutdown)
	{
		auto* resource = Graphics::D3D12::Helper::CreateBuffer(Buffer.data(), (uint32)Buffer.size());
		Graphics::D3D12::Core::DeferredRelease(resource);
	}
}

template<class FuncPtr, class... Args>
void InitTestWorkers(FuncPtr&& funcPtr, Args&&... args)
{
#if ENABLE_TEST_WORKERS
	Shutdown = false;
	for (auto& w : Workers)
	{
		w = std::thread(std::forward<FuncPtr>(funcPtr), std::forward<Args>(args)...);
	}
#endif
}

void JointTestWorkers()
{
#if ENABLE_TEST_WORKERS
	Shutdown = true;
	for (auto& w : Workers)
	{
		w.join();
	}
#endif
}

ID::IDType ModelID{ ID::INVALID_ID };

Graphics::RenderSurface Surfaces[4];

bool Resized = false;
bool IsRestarting = false;

bool TestInitialize();
void TestShutdown();

void DestroyRenderSurface(Graphics::RenderSurface& renderSurface);

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool toggleFullscreen = false;

	switch (msg)
	{
	case WM_DESTROY:
	{
		bool allClosed = true;
		for (uint32 i = 0; i < _countof(Surfaces); i++)
		{
			if (Surfaces[i].Window.IsValid())
			{
				if (Surfaces[i].Window.IsClosed())
				{
					DestroyRenderSurface(Surfaces[i]);
				}
				else
				{
					allClosed = false;
				}
			}
			
		}
		if (allClosed && !IsRestarting)
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	break;
	case WM_SIZE:
		Resized = (wparam != SIZE_MINIMIZED);
		break;
	case WM_SYSCHAR:
		toggleFullscreen = (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN));
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
		else if (wparam == VK_F11)
		{
			IsRestarting = true;
			TestShutdown();
			TestInitialize();
		}
	}

	if ((Resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggleFullscreen)
	{
		Platform::Window win{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd,GWLP_USERDATA)} };
		for (uint32 i = 0; i < _countof(Surfaces); i++)
		{
			if (win.ID() == Surfaces[i].Window.ID())
			{
				if (toggleFullscreen)
				{
					win.SetFullScreen(!win.IsFullScreen());
					return 0;
				}
				else
				{
					Surfaces[i].Surface.Resize(win.Width(), win.Height());
					Resized = false;
				}
				break;
			}

		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool ReadFile(std::filesystem::path path, OUT std::unique_ptr<uint8[]>& data, OUT uint64& size)
{
	if (!std::filesystem::exists(path))
		return false;

	size = std::filesystem::file_size(path);
	assert(size);
	if (!size)
		return false;

	data = std::make_unique<uint8[]>(size);
	std::ifstream file{ path, std::ios::in | std::ios::binary };
	if (!file || !file.read((char*)data.get(), size))
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

void CreateRenderSurface(Graphics::RenderSurface& renderSurface, Platform::WindowInitInfo info)
{
	renderSurface.Window = Platform::CreateMyWindow(&info);
	renderSurface.Surface = Graphics::CreateSurface(renderSurface.Window);
}

void DestroyRenderSurface(Graphics::RenderSurface& renderSurface)
{
	Graphics::RenderSurface tmp{ renderSurface };
	renderSurface = {};
	if (tmp.Surface.IsValid())
	{
		Graphics::RemoveSurface(tmp.Surface.ID());
	}
	if (tmp.Window.IsValid())
	{
		Platform::RemoveMyWindow(tmp.Window.ID());
	}
}

bool TestInitialize()
{
	while (!CompileShaders())
	{
		if (MessageBox(nullptr, L"エンジンシェーダーをコンパイルできませんでした", L"Shader Compile Error", MB_RETRYCANCEL) != IDRETRY)
			return false;
	}

	if (!Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12))
		return false;

	Platform::WindowInitInfo info[]
	{
		{&WinProc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&WinProc, nullptr, L"Render Window 2", 150, 150, 400, 800},
		{&WinProc, nullptr, L"Render Window 3", 200, 200, 400, 800},
		{&WinProc, nullptr, L"Render Window 4", 250, 250, 400, 800}
	};
	static_assert(_countof(info) == _countof(Surfaces));

	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		CreateRenderSurface(Surfaces[i], info[i]);
	}

	// テストモデルのロード
	std::unique_ptr<uint8[]> model;
	uint64 size = 0;
	if (!ReadFile("..\\..\\Test\\test.model", model, size))
		return false;

	ModelID = Content::CreateResource(model.get(), Content::AssetType::Mesh);
	if (!ID::IsValid(ModelID))
		return false;

	InitTestWorkers(BufferTestWorker);

	IsRestarting = false;
	return true;
}

bool EngineTest::Initialize()
{
	return TestInitialize();
}

void EngineTest::Run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		if (Surfaces[i].Surface.IsValid())
		{
			Surfaces[i].Surface.Render();
		}
	}
}

void TestShutdown()
{
	JointTestWorkers();

	if (ID::IsValid(ModelID))
	{
		Content::DestroyResource(ModelID, Content::AssetType::Mesh);
	}

	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		DestroyRenderSurface(Surfaces[i]);
	}
	Graphics::Shutdown();
}

void EngineTest::Shutdown()
{
	TestShutdown();
}

#endif // TEST_RENDERER