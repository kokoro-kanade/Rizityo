#pragma once
#include "ToolCommon.h"

namespace Rizityo::AssetTool
{
	namespace PackedVertex
	{
		struct VertexStatic
		{
			Math::Vector3 Position;
			uint8 Reserved[3];
			uint8 tSign; // bit 0; tangent handedness * (tangent.x sign), bit 1: �@����z�����̕��� (0 -> -1, 1 -> +1)
			uint16 Normal[2]; // �@���͐��K������Ă���̂�x,y�ƕ����̏�񂩂�z�����߂���
			uint16 Tangent[2];
			Math::Vector2 UV;
		};
	}

	struct Vertex
	{
		Math::Vector4 Tangent{};
		Math::Vector3 Position{};
		Math::Vector3 Normal{};
		Math::Vector2 UV{};
	};

	struct Mesh
	{
		// �������f�[�^
		Utility::Vector<Math::Vector3> Positions;
		Utility::Vector<Math::Vector3> Normals;
		Utility::Vector<Math::Vector4> Tangents;
		Utility::Vector<Utility::Vector<Math::Vector2>> UVSets;
		Utility::Vector<uint32> RawIndices;

		// ���ԃf�[�^
		Utility::Vector<Vertex> Vertices;
		Utility::Vector<uint32> Indices;

		// �o�̓f�[�^
		std::string Name;
		Utility::Vector<PackedVertex::VertexStatic> PackedVerticesStatic;
		float32 LodThreshold = { -1.f };
		uint32 LodId = UINT32_INVALID_NUM;
	};

	struct LodGroup
	{
		std::string Name;
		Utility::Vector<Mesh> Meshes;
	};

	struct Level
	{
		std::string Name;
		Utility::Vector<LodGroup> LodGroups;
	};

	struct GeometryImportSetting
	{
		float32 SmoothingAngle; // �Ȃ����Ă���ʂ��Ȃ߂炩�ł��镽�ʊԂ̊p�x��臒l
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

	void ProcessLevel(Level& level, const GeometryImportSetting& setting); // �G���W���p�Ƀf�[�^��ϊ�
	void PackData(const Level& level, LevelData& data); // �G�f�B�^�Ƀf�[�^��n�����߂Ƀp�b�L���O
}