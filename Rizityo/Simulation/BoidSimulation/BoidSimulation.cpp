#include "Components/Entity.h"
#include "Components/Transform.h"
#include "Components/Script.h"
#include "Components/Render.h"
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
	constexpr float32 WallDistance = 5.f;
	constexpr float32 WallForceScale = 2.f;

	constexpr Math::Vector3 LightPos{ -10.f, 10.f, 0.f };
	constexpr Math::Vector3 LightRot{ Math::HALF_PI / 2, 0, 0 };
	constexpr Math::Vector3 LightColor{ 174.f / 255.f, 174.f / 255.f, 174.f / 255.f };
}

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

	// 壁
	GameEntity::Entity WallEntities[WallNum]{};

	// ライト
	constexpr uint32 LightSetKey = 0;
	Graphics::Light WorldLight{};
	GameEntity::Entity LightEntity{};

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
}

void BoidSimulation::Initialize()
{
	LoadContents();
	CreateWorld();
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
	RemoveWorld();
	UnloadContents();
}


