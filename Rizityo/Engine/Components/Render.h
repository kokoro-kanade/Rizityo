#pragma once
#include "ComponentsCommonHeaders.h"
#include "API/RenderComponent.h"
#include "Graphics/Renderer.h"

namespace Rizityo::Render
{
	struct ShaderType
	{
		enum Type : uint32
		{
			Vertex = 0,
			Hull,
			Domain,
			Geometry,
			Pixel,
			Compute,
			Amplification,
			Mesh,
			Count
		};
	};

	struct ShaderInfo
	{
		const char* FileName;
		const char* FunctionName;
	};

	struct MaterialInfo
	{
		const char* MaterialName;
		ShaderInfo ShadersInfo[ShaderType::Count];
		Graphics::MaterialType::Type Type;
		// TODO : テクスチャ
	};

	struct InitInfo
	{
		const char* ModelFilePath;
		uint32 MaterialCount;
		Vector<MaterialInfo> MaterialsInfo; // TODO : 性能的にMaterialInfo*にする
	};

	Render::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity);
	void RemoveComponent(Render::Component component);

	void Update(); // thresholdsの更新

	void GetRenderFrameInfo(OUT Graphics::FrameInfo& info);

	void AddShaderID(const char* fileName, const char* functionName, ID::IDType vsID);

	void RemoveShaderID(const char* fileName, const char* functionName);
	void RemoveModel(const char* modelFilePath);
	void RemoveMaterial(const char* materialName);
}
