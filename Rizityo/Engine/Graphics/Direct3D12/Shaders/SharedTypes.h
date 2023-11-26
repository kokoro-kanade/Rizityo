#pragma once
#include "Graphics/Direct3D12/D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::HLSL {

	using float4x4 = Math::Matrix4x4a;
	using float4 = Math::Vector4;
	using float3 = Math::Vector3;
	using float2 = Math::Vector2;
	using uint4 = Math::U32Vector4;
	using uint3 = Math::U32Vector3;
	using uint2 = Math::U32Vector2;
	using uint = uint32;

#include "CommonTypes.hlsli"
}