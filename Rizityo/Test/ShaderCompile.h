#pragma once
#include "CommonHeaders.h"

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

struct ShaderFileInfo
{
	const char* FileName;
	const char* FunctionName;
	ShaderType::Type Type;
};

std::unique_ptr<uint8[]> CompileShader(ShaderFileInfo info, const char* filePath);

bool CompileShaders();