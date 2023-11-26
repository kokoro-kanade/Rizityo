#include "TestRenderer.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Platform.h"
#include "Graphics/Renderer.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Content/ContentToEngine.h"
#include "Components/Entity.h"
#include "Components/Transform.h"
#include "ShaderCompile.h"
#include <filesystem>
#include <fstream>

#if TEST_RENDERER

using namespace Rizityo;

#define ENABLE_TEST_WORKERS 0

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

ID::IDType TestModelID{ ID::INVALID_ID };
ID::IDType TestItemID{ ID::INVALID_ID };

struct TestSurface
{
	Graphics::RenderSurface Surface;
	GameEntity::Entity Entity{};
	Graphics::Camera Camera{};
};

TestSurface Surfaces[4];

bool Resized = false;
bool IsRestarting = false;

bool TestInitialize();
void TestShutdown();

void DestroyTestSurface(OUT TestSurface& testSurface);

ID::IDType CreateRenderItem(ID::IDType entityId);
void DestroyRenderItem(ID::IDType itemId);

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
			if (Surfaces[i].Surface.Window.IsValid())
			{
				if (Surfaces[i].Surface.Window.IsClosed())
				{
					DestroyTestSurface(Surfaces[i]);
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
			if (win.ID() == Surfaces[i].Surface.Window.ID())
			{
				if (toggleFullscreen)
				{
					win.SetFullScreen(!win.IsFullScreen());
					return 0;
				}
				else
				{
					Surfaces[i].Surface.Surface.Resize(win.Width(), win.Height());
					Surfaces[i].Camera.SetAspectRatio((float32)win.Width() / win.Height());
					Resized = false;
				}
				break;
			}

		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

GameEntity::Entity CreateTestGameEntity(bool isCamera)
{
	Transform::InitInfo transformInfo{};
	Math::Vector3a rot{ 0, isCamera ? 3.14f : 0.f, 0 };
	DirectX::XMVECTOR quat{ DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3A(&rot)) };
	Math::Vector4a rotQuat;
	DirectX::XMStoreFloat4A(&rotQuat, quat);
	memcpy(&transformInfo.Rotation[0], &rotQuat.x, sizeof(transformInfo.Rotation));

	if (isCamera)
	{
		transformInfo.Position[1] = 1.f;
		transformInfo.Position[2] = 3.f;
	}

	GameEntity::EntityInfo entityInfo{};
	entityInfo.Transform = &transformInfo;
	GameEntity::Entity entity{ GameEntity::CreateGameEntity(entityInfo) };
	assert(entity.IsValid());
	return entity;
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

void CreateTestSurface(OUT TestSurface& testSurface, Platform::WindowInitInfo info)
{
	testSurface.Surface.Window = Platform::CreateMyWindow(&info);
	testSurface.Surface.Surface = Graphics::CreateSurface(testSurface.Surface.Window);
	testSurface.Entity = CreateTestGameEntity(true);
	testSurface.Camera = Graphics::CreateCamera(Graphics::PerspectiveCameraInitInfo{ testSurface.Entity.ID() });
	testSurface.Camera.SetAspectRatio((float32)testSurface.Surface.Window.Width() / testSurface.Surface.Window.Height());
}

void DestroyTestSurface(OUT TestSurface& testSurface)
{
	TestSurface tmp{ testSurface };
	testSurface = {};
	if (tmp.Surface.Surface.IsValid())
	{
		Graphics::RemoveSurface(tmp.Surface.Surface.ID());
	}
	if (tmp.Surface.Window.IsValid())
	{
		Platform::RemoveMyWindow(tmp.Surface.Window.ID());
	}
	if (tmp.Camera.IsValid())
	{
		Graphics::RemoveCamera(tmp.Camera.ID());
	}
	if (tmp.Entity.IsValid())
	{
		GameEntity::RemoveGameEnity(tmp.Entity.ID());
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
		CreateTestSurface(Surfaces[i], info[i]);
	}

	// テストモデルのロード
	std::unique_ptr<uint8[]> model;
	uint64 size = 0;
	if (!ReadFile("..\\..\\Test\\test.model", model, size))
		return false;

	TestModelID = Content::CreateResource(model.get(), Content::AssetType::Mesh);
	if (!ID::IsValid(TestModelID))
		return false;

	InitTestWorkers(BufferTestWorker);

	TestItemID = CreateRenderItem(CreateTestGameEntity(false).ID());

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
		if (Surfaces[i].Surface.Surface.IsValid())
		{
			float32 threshold = 10;
			Surfaces[i].Surface.Surface.Render({&TestItemID, &threshold, 1, Surfaces[i].Camera.ID()});
		}
	}
}

void TestShutdown()
{
	DestroyRenderItem(TestItemID);

	JointTestWorkers();

	if (ID::IsValid(TestModelID))
	{
		Content::DestroyResource(TestModelID, Content::AssetType::Mesh);
	}

	for (uint32 i = 0; i < _countof(Surfaces); i++)
	{
		DestroyTestSurface(Surfaces[i]);
	}
	Graphics::Shutdown();
}

void EngineTest::Shutdown()
{
	TestShutdown();
}

#endif // TEST_RENDERER