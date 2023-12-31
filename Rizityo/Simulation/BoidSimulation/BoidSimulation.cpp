#include "Components/Entity.h"
#include "Components/Transform.h"
#include "Components/Script.h"
#include "Components/Render.h"
#include "API/Input.h"
#include "API/Light.h"
#include "BoidSimulation.h"
#include "Boid.h"

using namespace Rizityo;

namespace
{
	constexpr uint32 BoidXNum = 10;
	constexpr uint32 BoidZNum = 10;
	constexpr uint32 BoidNum = BoidXNum * BoidZNum;

	constexpr uint32 WallNum = 4;
	constexpr float32 WallLeftX = -15.f;
	constexpr float32 WallRightX = 15.f;
	constexpr float32 WallBackZ = -15.f;
	constexpr float32 WallForwardZ = 15.f;

	constexpr Math::Vector3 LightPos{ -10.f, 10.f, 0.f };
	constexpr Math::Vector3 LightRot{ Math::HALF_PI / 2, 0, 0 };
	constexpr Math::Vector3 LightColor{ 174.f / 255.f, 174.f / 255.f, 174.f / 255.f };

} // 定数

namespace
{	
	// 初期化情報
	Transform::InitInfo BoidTransformInfo{};
	Transform::InitInfo WallTransformInfo{};
	Script::InitInfo BoidScriptInfo{};
	Render::MaterialInfo BoidMaterialInfo{};
	Render::InitInfo BoidRenderInfo{};
	Render::InitInfo WallRenderInfo{};
	GameEntity::InitInfo BoidInfo{ &BoidTransformInfo, &BoidScriptInfo, &BoidRenderInfo };
	GameEntity::InitInfo WallInfo{ &WallTransformInfo, nullptr, &WallRenderInfo };

	// ボイド
	GameEntity::Entity BoidEntities[BoidNum]{};
	Math::Vector3 BoidPositions[BoidNum]{};
	Math::Vector3 BoidVerocities[BoidNum]{};
	std::unordered_map<ID::IDType, uint32> BoidEntityID_IndexMapping;

	float32 AlignementWeight = 1.f;
	float32 CohesionWeight = 1.f;
	float32 SeperationWeight = 1.f;
	float32 NeighborRadius = 4.f;
	float32 SeperationRadius = 3.f;
	float32 FOV = 20.f;
	bool UpdateFlag = true;

	// 壁
	GameEntity::Entity WallEntities[WallNum]{};
	float32 WallForceScale = 2.5f;
	float32 WallDistance = 5.f;

	// ライト
	constexpr uint32 LightSetKey = 0;
	Graphics::Light WorldLight{};
	GameEntity::Entity LightEntity{};

	// UI
	BoidSimulationGUI SimUI{"Boid Parameter", 500, 300};
	bool Simulating = false;

}

namespace
{
	void LoadContents()
	{
		BoidScriptInfo.CreateFunc = Script::Internal::GetScriptCreateFunc(Script::Internal::StringHash()("BoidScript"));
		BoidRenderInfo.ModelFilePath = "..\\..\\Content\\Model\\boid.model";
		BoidRenderInfo.ModelName = "BoidModel";
		BoidRenderInfo.MaterialCount = 1;
		BoidMaterialInfo.MaterialName = "BoidMaterial";
		BoidMaterialInfo.ShadersInfo[Render::ShaderType::Vertex] = { "Shader.hlsl", "ShaderVS" };
		BoidMaterialInfo.ShadersInfo[Render::ShaderType::Pixel] = { "Shader.hlsl", "ShaderPS" };
		BoidMaterialInfo.Type = Graphics::MaterialType::Opaque;
		for (uint32 i = 0; i < BoidRenderInfo.MaterialCount; i++)
		{
			BoidRenderInfo.MaterialsInfo.push_back(BoidMaterialInfo);
		}
		GameEntity::Internal::RegisterEntity("Boid", &BoidInfo);

		WallRenderInfo.ModelFilePath = "..\\..\\Content\\Model\\wall.model";
		WallRenderInfo.ModelName = "WallModel";
		WallRenderInfo.MaterialCount = 1;
		for (uint32 i = 0; i < WallRenderInfo.MaterialCount; i++)
		{
			WallRenderInfo.MaterialsInfo.push_back(BoidMaterialInfo);
		}
		GameEntity::Internal::RegisterEntity("Wall", &WallInfo);
	}

	void UnloadContents()
	{
		Render::RemoveModel("WallModel");
		Render::RemoveMaterial("BoidMaterial");
		Render::RemoveModel("BoidModel");
	}

	void CreateBoids()
	{
		for (uint32 i = 0; i < BoidXNum; i++)
		{
			for (uint32 j = 0; j < BoidZNum; j++)
			{
				const GameEntity::Entity entity = GameEntity::Spawn("Boid", { static_cast<float32>(i), 0.f, static_cast<float32>(j) });
				BoidEntities[i * BoidZNum + j] = entity;
				BoidEntityID_IndexMapping[entity.ID()] = i * BoidZNum + j;
			}
		}
	}

	void RemoveBoids()
	{
		for (uint32 i = 0; i < BoidNum; i++)
		{
			GameEntity::RemoveGameEnity(BoidEntities[i].ID());
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
		CreateBoids();
		CreateLight();
	}

	void RemoveWorld()
	{
		RemoveLight();
		RemoveBoids();
		RemoveWalls();
	}
}

namespace Boid
{
	uint32 GetBoidNum()
	{
		return BoidNum;
	}

	const GameEntity::Entity* const GetBoidEntity()
	{
		return &BoidEntities[0];
	}

	const Math::Vector3* const GetBoidPositions()
	{
		return &BoidPositions[0];
	}

	const Math::Vector3* const GetBoidVerocities()
	{
		return &BoidVerocities[0];
	}

	uint32 GetEntityIndex(ID::IDType id)
	{
		return BoidEntityID_IndexMapping[id];
	}

	Math::Vector3 CalcWallForce(Math::Vector3 pos)
	{
		using namespace Math;
		Vector3 force{ Vector3::ZERO };

		if (pos.x - WallLeftX < WallDistance)
			force += abs(pos.x - WallLeftX) * WallForceScale * Vector3::RIGHT;
		if (WallRightX - pos.x < WallDistance)
			force += -abs(WallRightX - pos.x) * WallForceScale * Vector3::RIGHT;
		if (pos.z - WallBackZ < WallDistance)
			force += abs(pos.z - WallBackZ) * WallForceScale * Vector3::FORWARD;
		if (WallForwardZ - pos.z < WallDistance)
			force += -abs(WallForwardZ - pos.z) * WallForceScale * Vector3::FORWARD;

		return force;
	}

	float32 GetAlignement() { return AlignementWeight; }
	float32 GetCohesion() { return CohesionWeight; }
	float32 GetSeperation() { return SeperationWeight; }
	float32 GetNeighborRadius() { return NeighborRadius; }
	float32 GetSeperationRadius() { return SeperationRadius; }
	float32 GetFOV() { return FOV; }
	bool GetUpdateFlag() { return UpdateFlag; }
}

void BoidSimulation::Initialize()
{
	LoadContents();
	CreateWorld();
	Simulating = true;
	SimUI.SetFlag(true);
}

void BoidSimulation::Update()
{
	for (uint32 i = 0; i < BoidNum; i++)
	{
		BoidPositions[i] = BoidEntities[i].GetPosition();
		BoidVerocities[i] = BoidEntities[i].GetScriptComponent().GetScript<BoidScript>()->GetVerocity();
	}
}

void BoidSimulation::Shutdown()
{
	SimUI.SetFlag(false);
	Simulating = false;
	RemoveWorld();
	UnloadContents();
}

void BoidSimulationGUI::ShowContent()
{
	// 各種パラメータ(boid)
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode("Boid Parameters"))
	{
		ImGui::SliderFloat("Alignement Weight", &AlignementWeight, 0.f, 5.f);
		ImGui::SliderFloat("Cohesion Weight", &CohesionWeight, 0.f, 5.f);
		ImGui::SliderFloat("Seperation Weight", &SeperationWeight, 0.f, 5.f);
		ImGui::SliderFloat("Neighbor Radius", &NeighborRadius, 0.f, 5.f);
		ImGui::SliderFloat("Seperation Radius", &SeperationRadius, 0.f, 5.f);
		ImGui::SliderFloat("FOV", &FOV, 0.f, 45.f);

		ImGui::TreePop();
	}

	ImGui::Dummy({ 0.f, 20.f });
	
	// スタート/ストップ
	bool button = false;
	button = ImGui::Button("Stop", {60, 40});
	if (button) UpdateFlag = false;
	ImGui::SameLine();
	ImGui::Dummy({ 5.f, 0 });
	ImGui::SameLine();
	button = ImGui::Button("Start", {60, 40});
	if(button) UpdateFlag = true;

}

void BoidSimulationGUI::Update(float32 dt)
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


