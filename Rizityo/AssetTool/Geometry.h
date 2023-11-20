#pragma once
#include "ToolCommon.h"

namespace Rizityo::AssetTool
{
    namespace Elements {

        struct ElementsType {
            enum Type : uint32 {
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
            Math::Vector2 UV;
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
            Math::Vector2    UV;
        };

        struct SkeletalNormalTextureColor
        {
            uint8 JointWeights[3];
            uint8 TSign;          
            uint16 JointIndices[4];
            uint16 Normal[2];
            uint16 Tangent[2];
            Math::Vector2 UV;
            uint8 Color[3];
            uint8 Padding;
        };

    } // namespace Elements

    struct Vertex
    {
        Math::Vector4 Tangent{};
        Math::Vector4 JointWeights{};
        Math::U32Vector4 JointIndices{ UINT32_INVALID_NUM, UINT32_INVALID_NUM , UINT32_INVALID_NUM , UINT32_INVALID_NUM };
        Math::Vector3 Position{};
        Math::Vector3 Normal{};
        Math::Vector2 UV{};
        uint8 Red{}, Green{}, Blue{};
        uint8 Padding;
    };

	struct Mesh
	{
		// 初期化データ
		Utility::Vector<Math::Vector3> Positions;
        Utility::Vector<Math::Vector3> Colors;
		Utility::Vector<Math::Vector3> Normals;
		Utility::Vector<Math::Vector4> Tangents;
		Utility::Vector<Utility::Vector<Math::Vector2>> UVSets;
		Utility::Vector<uint32> MaterialIndices;
		Utility::Vector<uint32> MaterialUsed;
		Utility::Vector<uint32> RawIndices;

		// 中間データ
		Utility::Vector<Vertex> Vertices;
		Utility::Vector<uint32> Indices;

		// 出力データ
        Elements::ElementsType::Type ElementsType;
        Utility::Vector<uint8>  PositionBuffer;
        Utility::Vector<uint8>  ElementBuffer;

		std::string Name;
		float32 LODThreshold = { -1.f };
		uint32 LOD_ID = UINT32_INVALID_NUM;
	};

	struct LODGroup
	{
		std::string Name;
		Utility::Vector<Mesh> Meshes;
	};

	struct Level
	{
		std::string Name;
		Utility::Vector<LODGroup> LODGroups;
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