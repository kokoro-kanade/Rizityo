#include "SynchroSimulation.h"
#include "Components/Entity.h"
#include "Components/Transform.h"
#include "Components/Script.h"
#include "Components/Render.h"
#include "API/Input.h"
#include "API/Light.h"
#include "Oscillator.h"

using namespace Rizityo;

namespace
{
	constexpr uint32 OscillatorXNum = 10;
	constexpr uint32 OscillatorZNum = 10;
	constexpr uint32 OscillatorNum = OscillatorXNum * OscillatorZNum;
	constexpr float32 IntervalScale = 2.f;

	constexpr uint32 WallNum = 4;
	constexpr float32 WallLeftX = -15.f;
	constexpr float32 WallRightX = 15.f;
	constexpr float32 WallBackZ = -15.f;
	constexpr float32 WallForwardZ = 15.f;
	constexpr float32 WallDistance = 5.f;
	constexpr float32 WallForceScale = 2.f;

	constexpr Math::Vector3 LightPos{ -10.f, 10.f, 0.f };
	constexpr Math::Vector3 LightRot{ Math::HALF_PI / 2, 0, 0 };
	constexpr Math::Vector3 LightColor{ 174.f / 255.f, 174.f / 255.f, 174.f / 255.f };
}

namespace
{
	// 初期化情報
	Transform::InitInfo OscillatorTransformInfo{};
	Transform::InitInfo WallTransformInfo{};
	Script::InitInfo OscillatorScriptInfo{};
	Render::MaterialInfo OscillatorMaterialInfo{};
	Render::InitInfo OscillatorRenderInfo{};
	Render::InitInfo WallRenderInfo{};
	GameEntity::InitInfo OscillatorInfo{ &OscillatorTransformInfo, &OscillatorScriptInfo, &OscillatorRenderInfo };
	GameEntity::InitInfo WallInfo{ &WallTransformInfo, nullptr, &WallRenderInfo };

	// オシレーター
	GameEntity::Entity OscillatorEntities[OscillatorNum]{};
	Math::Vector3 OscillatorPositions[OscillatorNum]{};
	float32 OscillatorPhases[OscillatorNum]{};
	std::unordered_map<ID::IDType, uint32> OscillatorEntityID_IndexMapping;

	float32 Speed = 2.f; // 移動スピード
	float32 NeighborRadius = 4.f; // 近接領域の半径(m)
	float32 Weight = 1.f; // 位相更新の重み
	bool UpdateFlag = true;

	// 壁
	GameEntity::Entity WallEntities[WallNum]{};

	// ライト
	constexpr uint32 LightSetKey = 0;
	Graphics::Light WorldLight{};
	GameEntity::Entity LightEntity{};

	// UI
	SynchroSimulationGUI SimUI{ "Oscillator Parameter", 500, 300 };
	bool Simulating = false;
}

namespace
{
	void LoadContents()
	{
		OscillatorScriptInfo.CreateFunc = Script::Internal::GetScriptCreateFunc(Script::Internal::StringHash()("OscillatorScript"));
		OscillatorRenderInfo.ModelFilePath = "..\\..\\Content\\Model\\oscillator.model";
		OscillatorRenderInfo.ModelName = "OscillatorModel";
		OscillatorRenderInfo.MaterialCount = 1;
		OscillatorMaterialInfo.MaterialName = "OscillatorMaterial";
		OscillatorMaterialInfo.ShadersInfo[Render::ShaderType::Vertex] = { "Shader.hlsl", "ShaderVS" };
		OscillatorMaterialInfo.ShadersInfo[Render::ShaderType::Pixel] = { "Shader.hlsl", "ShaderPS" };
		OscillatorMaterialInfo.Type = Graphics::MaterialType::Opaque;
		for (uint32 i = 0; i < OscillatorRenderInfo.MaterialCount; i++)
		{
			OscillatorRenderInfo.MaterialsInfo.push_back(OscillatorMaterialInfo);
		}
		GameEntity::Internal::RegisterEntity("Oscillator", &OscillatorInfo);

		WallRenderInfo.ModelFilePath = "..\\..\\Content\\Model\\wall.model";
		WallRenderInfo.ModelName = "WallModel";
		WallRenderInfo.MaterialCount = 1;
		for (uint32 i = 0; i < WallRenderInfo.MaterialCount; i++)
		{
			WallRenderInfo.MaterialsInfo.push_back(OscillatorMaterialInfo);
		}
		GameEntity::Internal::RegisterEntity("Wall", &WallInfo);
	}

	void UnloadContents()
	{
		Render::RemoveModel("WallModel");
		Render::RemoveMaterial("OscillatorMaterial");
		Render::RemoveModel("OscillatorModel");
	}

	void CreateOscillators()
	{
		constexpr float32 xOffset = IntervalScale * static_cast<float32>(OscillatorXNum - 1) / 2;
		constexpr float32 zOffset = IntervalScale * static_cast<float32>(OscillatorZNum - 1) / 2;
		for (uint32 i = 0; i < OscillatorXNum; i++)
		{
			for (uint32 j = 0; j < OscillatorZNum; j++)
			{
				const GameEntity::Entity entity = GameEntity::Spawn("Oscillator", { IntervalScale * static_cast<float32>(i) - xOffset, 0.f, IntervalScale * static_cast<float32>(j) - zOffset });
				OscillatorEntities[i * OscillatorZNum + j] = entity;
				OscillatorEntityID_IndexMapping[entity.ID()] = i * OscillatorZNum + j;
			}
		}
	}

	void RemoveOscillators()
	{
		for (uint32 i = 0; i < OscillatorNum; i++)
		{
			GameEntity::RemoveGameEnity(OscillatorEntities[i].ID());
		}
	}

	void CreateWalls()
	{
		WallEntities[0] = GameEntity::Spawn("Wall", { 0.f, 0.f, WallBackZ - 1 }, { 0.f, 0.f, 0.f });
		WallEntities[1] = GameEntity::Spawn("Wall", { 0.f, 0.f, WallForwardZ + 1 }, { 0.f, 0.f, 0.f });
		WallEntities[2] = GameEntity::Spawn("Wall", { WallLeftX - 1, 0.f, 0.f }, { 0.f, Math::HALF_PI, 0.f });
		WallEntities[3] = GameEntity::Spawn("Wall", { WallRightX + 1, 0.f, 0.f }, { 0.f, Math::HALF_PI, 0.f });
	}

	void RemoveWalls()
	{
		for (uint32 i = 0; i < WallNum; i++)
		{
			GameEntity::RemoveGameEnity(WallEntities[i].ID());
		}
	}

	void CreateLight()
	{
		Graphics::LightInitInfo info{};

		Transform::InitInfo tInfo{};
		memcpy(&tInfo.Position[0], &LightPos.x, sizeof(tInfo.Position));
		Math::Quaternion rot{ LightRot };
		memcpy(&tInfo.Rotation[0], &rot.x, sizeof(tInfo.Rotation));
		GameEntity::InitInfo eInfo{};
		eInfo.Transform = &tInfo;
		LightEntity = GameEntity::CreateGameEntity(eInfo);
		assert(LightEntity.IsValid());

		info.EntityID = LightEntity.ID();
		info.Type = Graphics::Light::Directional;
		info.LightSetKey = LightSetKey;
		info.Intensity = 1.f;
		info.Color = LightColor;

		WorldLight = Graphics::CreateLight(info);
	}

	void RemoveLight()
	{
		Graphics::RemoveLight(WorldLight.ID(), WorldLight.LightSetKey());
		GameEntity::RemoveGameEnity(LightEntity.ID());
	}

	void CreateWorld()
	{
		CreateWalls();
		CreateOscillators();
		CreateLight();
	}

	void RemoveWorld()
	{
		RemoveLight();
		RemoveOscillators();
		RemoveWalls();
	}
}

namespace Oscillator
{
	uint32 GetOscillatorNum()
	{
		return OscillatorNum;
	}

	const GameEntity::Entity* const GetOscillatorEntity()
	{
		return &OscillatorEntities[0];
	}

	const Math::Vector3* const GetOscillatorPositions()
	{
		return &OscillatorPositions[0];
	}

	const float32* const GetOscillatorPhases()
	{
		return &OscillatorPhases[0];
	}

	uint32 GetEntityIndex(ID::IDType id)
	{
		return OscillatorEntityID_IndexMapping[id];
	}

	void ApplyWallCondition(OUT Math::Vector3& pos)
	{
		using namespace Math;

		if (pos.x < WallLeftX)
			pos.x += (WallRightX - WallLeftX);
		if (WallRightX < pos.x)
			pos.x -= (WallRightX - WallLeftX);
		if (pos.z < WallBackZ)
			pos.z += (WallForwardZ - WallBackZ);
		if (WallForwardZ < pos.z)
			pos.z -= (WallForwardZ - WallBackZ);

	}

	float32 GetSpeed() { return Speed; }
	float32 GetNeighborRadius() { return NeighborRadius; }
	float32 GetWeight() { return Weight; }
	bool GetUpdateFlag() { return UpdateFlag; }
}

void SynchroSimulation::Initialize()
{
	LoadContents();
	CreateWorld();
	Simulating = true;
	SimUI.SetFlag(true);
}

void SynchroSimulation::Update()
{
	for (uint32 i = 0; i < OscillatorNum; i++)
	{
		OscillatorPositions[i] = OscillatorEntities[i].GetPosition();
		OscillatorPhases[i] = OscillatorEntities[i].GetScriptComponent().GetScript<OscillatorScript>()->GetPhase();
	}
}

void SynchroSimulation::Shutdown()
{
	SimUI.SetFlag(false);
	Simulating = false;
	RemoveWorld();
	UnloadContents();
}

void SynchroSimulationGUI::ShowContent()
{
	// 各種パラメータ(oscillator)
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode("Oscillator Parameters"))
	{
		ImGui::SliderFloat("Speed", &Speed, 0.f, 5.f);
		ImGui::SliderFloat("NeighborRadius", &NeighborRadius, 0.f, 5.f);
		ImGui::SliderFloat("Weight", &Weight, 0.f, 5.f);

		ImGui::TreePop();
	}

	ImGui::Dummy({ 0.f, 20.f });

	// スタート/ストップ
	bool button = false;
	button = ImGui::Button("Stop", { 60, 40 });
	if (button) UpdateFlag = false;
	ImGui::SameLine();
	ImGui::Dummy({ 5.f, 0 });
	ImGui::SameLine();
	button = ImGui::Button("Start", { 60, 40 });
	if (button) UpdateFlag = true;
}

// TODO? : この辺の処理をまとめる
// TODO? : Simulatingフラグを持つかGUIインスタンスをSimulationクラスのメンバに持つか
void SynchroSimulationGUI::Update(float32 dt)
{
	if (!Simulating)
		return;

	if (_IsCooling)
	{
		_ElapsedTime += dt;
		if (_ElapsedTime > _CoolTime)
		{
			_ElapsedTime = 0.f;
			_IsCooling = false;
		}

		return;
	}

	if (Input::GetKeyDown(Input::InputCode::Key0))
	{
		_ShowFlag = !_ShowFlag;
		_IsCooling = true;
	}
}