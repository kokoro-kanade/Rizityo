#pragma once
#include "ToolCommon.h"

namespace Rizityo::AssetTool
{
	enum class PrimitiveMeshType : uint32
	{
		Plane,
		Cube,
		UVSphere,
		IcoSphere,
		Cylinder,
		Capsule,
		Count
	};

	struct PrimitiveInitInfo
	{
		PrimitiveMeshType Type;
		uint32 Segments[3]{ 1,1,1 };
		Math::Vector3 Size{ 1,1,1 };
		uint32 Lod = 0;
	};

}