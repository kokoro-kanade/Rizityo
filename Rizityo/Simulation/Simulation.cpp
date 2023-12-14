#include "BoidSimulation/BoidSimulation.h"
#include "Core/Platform/PlatformWindow/PlatformWindow.h"
#include "Core/Utility/Time/Timer.h"
#include "Core/Utility/IO/FileIO.h"
#include "Graphics/Renderer.h"
#include "Components/Entity.h"
#include "Components/Transform.h"
#include "Components/Script.h"
#include "Components/Render.h"
#include "Input/Input.h"
#include "Content/AssetToEngine.h"


using namespace Rizityo;

namespace
{
	Time::Timer GameTimer{};

	struct SimulationWindow
	{
		Graphics::RenderSurface Surface;
		GameEntity::Entity Entity{};
		Graphics::Camera Camera{};
	};

	SimulationWindow Window{};

	constexpr Math::Vector3 CameraInitPos{ -10.f, 25.f, -10.f };
	constexpr Math::Vector3 CameraInitRot{ 1.f, Math::HALF_PI / 2, 0.f };
	constexpr const char* CameraScriptName = "CameraScript";

	Simulation* Sim = nullptr;

	BoidSimulation BoidSim{};

	bool Resized = false;
	bool IsRestarting = false;
}

bool Initialize();
void Shutdown();

namespace
{
	void DestroyGameWindow();

	LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		bool toggleFullscreen = false;

		switch (msg)
		{
		case WM_DESTROY:
		{
			bool allClosed = true;
			if (Window.Surface.Window.IsValid())
			{
				if (Window.Surface.Window.IsClosed())
				{
					DestroyGameWindow();
				}
				else
				{
					allClosed = false;
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
				Shutdown();
				Initialize();
			}
		}

		if ((Resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggleFullscreen)
		{
			Platform::Window win{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd,GWLP_USERDATA)} };
			if (win.ID() == Window.Surface.Window.ID())
			{
				if (toggleFullscreen)
				{
					win.SetFullScreen(!win.IsFullScreen());
					return 0;
				}
				else
				{
					Window.Surface.Surface.Resize(win.Width(), win.Height());
					Window.Camera.SetAspectRatio((float32)win.Width() / win.Height());
					Resized = false;
				}
			}

		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	void CreateGameWindow()
	{
		Platform::WindowInitInfo info
		{
			&WinProc, nullptr, L"Simulation"
		};
		Window.Surface.Window = Platform::CreateMyWindow(&info);
		Window.Surface.Surface = Graphics::CreateSurface(Window.Surface.Window);

		Transform::InitInfo tInfo{};
		memcpy(&tInfo.Position[0], &CameraInitPos.x, sizeof(tInfo.Position));
		Math::Quaternion rot{ CameraInitRot };
		memcpy(&tInfo.Rotation[0], &rot.x, sizeof(tInfo.Rotation));
		Script::InitInfo sInfo{};
		sInfo.CreateFunc = Script::Internal::GetScriptCreateFunc(Script::Internal::StringHash()(CameraScriptName));
		assert(sInfo.CreateFunc);
		GameEntity::InitInfo eInfo{};
		eInfo.Transform = &tInfo;
		eInfo.Script = &sInfo;
		Window.Entity = GameEntity::CreateGameEntity(eInfo);
		assert(Window.Entity.IsValid());

		Window.Camera = Graphics::CreateCamera(Graphics::PerspectiveCameraInitInfo{ Window.Entity.ID() });
		Window.Camera.SetAspectRatio(static_cast<float32>(Window.Surface.Window.Width() / Window.Surface.Window.Height()));
	}

	void DestroyGameWindow()
	{
		SimulationWindow win{ Window };
		Window = {};
		if (win.Surface.Surface.IsValid())
		{
			Graphics::RemoveSurface(win.Surface.Surface.ID());
		}
		if (win.Surface.Window.IsValid())
		{
			Platform::RemoveMyWindow(win.Surface.Window.ID());
		}
		if (win.Camera.IsValid())
		{
			Graphics::RemoveCamera(win.Camera.ID());
		}
		if (win.Entity.IsValid())
		{
			GameEntity::RemoveGameEnity(win.Entity.ID());
		}
	}

	void RenderWindow()
	{
		Graphics::FrameInfo info{};
		Render::GetRenderFrameInfo(info);
		info.CamerID = Window.Camera.ID();
		Window.Surface.Surface.Render(info);
	}

	void BindInput()
	{
		Input::InputSource source{};
		source.BindingKey = std::hash<std::string>()("Move");
		source.SourceType = Input::InputSource::Keyboard;
		source.Code = Input::InputCode::KeyA;
		source.Multiplier = 1.f;
		source.Axis = Input::Axis::X;
		Input::Bind(source);

		source.Code = Input::InputCode::KeyD;
		source.Multiplier = -1.f;
		Input::Bind(source);

		source.Code = Input::InputCode::KeyW;
		source.Multiplier = 1.f;
		source.Axis = Input::Axis::Z;
		Input::Bind(source);

		source.Code = Input::InputCode::KeyS;
		source.Multiplier = -1.f;
		Input::Bind(source);

		source.Code = Input::InputCode::KeyQ;
		source.Multiplier = -1.f;
		source.Axis = Input::Axis::Y;
		Input::Bind(source);

		source.Code = Input::InputCode::KeyE;
		source.Multiplier = 1.f;
		Input::Bind(source);
	}

	void LoadShader()
	{
		Vector<uint32> keys;
		keys.emplace_back(Content::VertexElementsType::StaticNormal);
		keys.emplace_back(Content::VertexElementsType::StaticNormalTexture);

		Vector<const uint8*> vs;
		std::unique_ptr<uint8[]> vs0;
		uint64 size;
		IO::ReadFile("..\\..\\Content\\Shader\\ShaderVS_SN.bin", vs0, size);
		vs.emplace_back(vs0.get());
		std::unique_ptr<uint8[]> vs1;
		IO::ReadFile("..\\..\\Content\\Shader\\ShaderVS_SNT.bin", vs1, size);
		vs.emplace_back(vs1.get());
		ID::IDType vsID = Content::AddShaderGroup(vs.data(), (uint32)vs.size(), keys.data());

		Render::AddShaderID("Shader.hlsl", "ShaderVS", vsID);

		std::unique_ptr<uint8[]> ps0;
		IO::ReadFile("..\\..\\Content\\Shader\\ShaderPS.bin", ps0, size);
		const uint8* ps[]{ ps0.get() };
		ID::IDType psID = Content::AddShaderGroup(&ps[0], 1, &UINT32_INVALID_NUM);

		Render::AddShaderID("Shader.hlsl", "ShaderPS", psID);
	}

	void UnloadShader()
	{
		Render::RemoveShader("Shader.hlsl", "ShaderVS");
		Render::RemoveShader("Shader.hlsl", "ShaderPS");
	}
}

bool Initialize()
{
	auto _ls = std::thread{ [] { LoadShader(); } };

	if (!Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12))
		return false;

	CreateGameWindow();

	BindInput();

	_ls.join();

	Sim = &BoidSim;
	Sim->Initialize();

	GameTimer.Reset();

	IsRestarting = false;

	return true;
}

void Update()
{
	GameTimer.Tick();

	// TODO : ボタンを押したら別のシミュレーション
	if (Input::GetKeyDown(Input::InputCode::Key1))
	{
		// Sim->Shutdown();
		// Sim = &BoidSim;
		// Sim->Initialize();
	}
	else if (Input::GetKeyDown(Input::InputCode::Key2))
	{

	}

	// std::this_thread::sleep_for(std::chrono::milliseconds(16));

	Sim->Update();
	Script::Update(GameTimer.DeltaTime());

	if (Window.Surface.Surface.IsValid())
	{
		RenderWindow();
	}

}

void Shutdown()
{
	Sim->Shutdown();

	DestroyGameWindow();

	Input::Unbind(std::hash<std::string>()("Move"));

	UnloadShader();

	Graphics::Shutdown();
}