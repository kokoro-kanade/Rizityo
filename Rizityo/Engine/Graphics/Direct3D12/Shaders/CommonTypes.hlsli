#if !defined(RIZITYO_COMMON_HLSLI) && !defined(__cplusplus)
#error ���̃t�@�C����Common.hlsli�o�R�ŃC���N���[�h���Ă�������
#endif

struct GlobalShaderData
{
    float4x4 View;
    float4x4 Projection;
    float4x4 InvProjection;
    float4x4 ViewProjection;
    float4x4 InvViewProjection;

    float3 CameraPosition;
    float ViewWidth;

    float3 CameraDirection;
    float ViewHeight;

    uint NumDirectionalLights;
    
    float DeltaTime;
};

struct PerObjectData
{
    float4x4 World;
    float4x4 InvWorld;
    float4x4 WorldViewProjection;
};

struct DirectionalLightParameters
{
    float3 Direction;
    float Intensity;
    float3 Color;
    float _Padding;
};

#ifdef __cplusplus
static_assert((sizeof(PerObjectData) % 16) == 0,
              "PerObjectData��16�o�C�g�̔{���ł���K�v������܂�");
static_assert((sizeof(DirectionalLightParameters) % 16) == 0,
              "DirectionalLightParameters��16�o�C�g�̔{���ł���K�v������܂�");
#endif