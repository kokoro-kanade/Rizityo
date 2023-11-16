#include "Fractals.hlsli"

struct ShaderConstants
{
	float Width;
	float Height;
	uint Frame;
};

ConstantBuffer<ShaderConstants> ShaderParams : register(b1);

#define NUMSAMPLE 4

float4 FillColorPS(in noperspective float4 Position : SV_Position, in noperspective float2 UV : TEXCOORD) : SV_Target0
{
	const float offset = 0.2f;
	const float2 offsets[4] =
	{
		float2(-offset, offset),
		float2(offset, offset),
		float2(offset, -offset),
		float2(-offset, -offset)
	};

	const float2 invDim = float2(1.f / ShaderParams.Width, 1.f / ShaderParams.Height);
	float3 color = 0.f;
	for (int i = 0; i < NUMSAMPLE; i++)
	{
		const float2 uv = (Position.xy + offsets[i]) * invDim;
		//color += DrawMandelbrot(uv);
		color += DrawJuliaSet(uv, ShaderParams.Frame);
	}

	return float4(color / NUMSAMPLE, 1.f);
}