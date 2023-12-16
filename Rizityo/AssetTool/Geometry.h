#pragma once
#include "ToolCommonHeader.h"

namespace Rizityo::AssetTool
{
    namespace Elements
    {
        struct ElementsType
        {
            enum Type : uint32
            {
                PositionOnly = 0x00,
                StaticNormal = 0x01,
                StaticNormalTexture = 0x03,
                StaticColor = 0x04,
                Skeletal = 0x08,
                SkeletalColor = Skeletal | StaticColor,
                SkeletalNormal = Skeletal | StaticNormal,
                SkeletalNormalColor = SkeletalNormal | StaticColor,
                SkeletalNormalTexture = Skeletal | StaticNormalTexture,
                SkeletalNormalTextureColor = SkeletalNormalTexture | StaticColor,
            };
        };

        struct StaticColor
        {
            uint8 Color[3];
            uint8 Padding;
        };

        struct StaticNormal
        {
            uint8 Color[3];
            uint8 TSign;     // bit 0: Tangent handedness * (Tangent.z sign), bit 1: Normal.z sign (0 means -1, 1 means +1).
            uint16 Normal[2];
        };

        struct StaticNormalTexture
        {
            uint8 Color[3];
            uint8 TSign;
            uint16 Normal[2];
            uint16 Tangent[2];
            Math::DX_Vector2 UV;
        };

        struct Skeletal
        {
            uint8 JointWeights[3]; // Normalized joint weights for up to 4 joints.
            uint8 Padding;
            uint16 JointIndices[4];
        };

        struct SkeletalColor
        {
            uint8 JointWeights[3];
            uint8 Padding;
            uint16 JointIndices[4];
            uint8 Color[3];
            uint8 Padding2;
        };

        struct SkeletalNormal
        {
            uint8 JointWeights[3];
            uint8 TSign;
            uint16 JointIndices[4];
            uint16 Normal[2];
        };

        struct SkeletalNormalColor
        {
            uint8 JointWeights[3];
            uint8 TSign;
            uint16 JointIndices[4];
            uint16 Normal[2];
            uint8 Color[3];
            uint8 Padding;
        };

        struct SkeletalNormalTexture
        {
            uint8 JointWeights[3];
            uint8 TSign;
            uint16 JointIndices[4];
            uint16 Normal[2];
            uint16 Tangent[2];
            Math::DX_Vector2    UV;
        };

        struct SkeletalNormalTextureColor
        {
            uint8 JointWeights[3];
            uint8 TSign;          
            uint16 JointIndices[4];
            uint16 Normal[2];
            uint16 Tangent[2];
            Math::DX_Vector2 UV;
            uint8 Color[3];
            uint8 Padding;
        };

    } // Elements

    struct Vertex
    {
        Math::DX_Vector4 Tangent{};
        Math::DX_Vector4 JointWeights{};
        Math::DX_U32Vector4 JointIndices{ UINT32_INVALID_NUM, UINT32_INVALID_NUM , UINT32_INVALID_NUM , UINT32_INVALID_NUM };
        Math::DX_Vector3 Position{};
        Math::DX_Vector3 Normal{};
        Math::DX_Vector2 UV{};
        uint8 Red{}, Green{}, Blue{};
        uint8 Padding;
    };

	struct Mesh
	{
		// 初期化データ
		Vector<Math::DX_Vector3> Positions;
        Vector<Math::DX_Vector3> Colors;
		Vector<Math::DX_Vector3> Normals;
		Vector<Math::DX_Vector4> Tangents;
		Vector<Vector<Math::DX_Vector2>> UVSets;
		Vector<uint32> MaterialIndices;
		Vector<uint32> MaterialUsed;
		Vector<uint32> RawIndices;

		// 中間データ
		Vector<Vertex> Vertices;
		Vector<uint32> Indices;

		// 出力データ
        Elements::ElementsType::Type ElementsType;
        Vector<uint8>  PositionBuffer;
        Vector<uint8>  ElementBuffer;

		std::string Name;
		float32 LODThreshold = { -1.f };
		uint32 LOD_ID = UINT32_INVALID_NUM;
	};

	struct LODGroup
	{
		std::string Name;
		Vector<Mesh> Meshes;
	};

	struct Level
	{
		std::string Name;
		Vector<LODGroup> LODGroups;
	};

	struct GeometryImportSetting
	{
		float32 SmoothingAngle; // つながっている面がなめらかである平面間の角度の閾値
		uint8 CalculateNormals;
		uint8 CalculateTangents;
		uint8 ReverseHandedness;
		uint8 ImportEmbededTextures;
		uint8 ImportAnimations;
	};

	struct LevelData
	{
		uint8* Data;
		uint32 DataSize;
		GeometryImportSetting Setting;
	};

	void ProcessLevel(Level& level, const GeometryImportSetting& setting); // エンジン用にデータを変換
	void PackData(const Level& level, LevelData& data); // エディタにデータを渡すためにパッキング
}