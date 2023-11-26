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
    uint ViewWidth;

    float3 CameraDirection;
    uint ViewHeight;

    float DeltaTime;
};

struct PerObjectData
{
    float4x4 World;
    float4x4 InvWorld;
    float4x4 WorldViewProjection;
};

#ifdef __cplusplus
static_assert((sizeof(PerObjectData) % 16) == 0,
              "PerObjectData��16�o�C�g�̔{���ł���K�v������܂�");
#endif