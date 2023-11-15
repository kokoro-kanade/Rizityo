#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Shader
{
	struct ShaderType {
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

	struct EngineShader
	{
		enum ID : uint32
		{
			FullScreenTriangleVS = 0,
			FillColorPS = 1,
			Count
		};
	};

	bool Initialize();
	void Shutdown();

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);

}