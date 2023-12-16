#pragma once
#include "ToolCommonHeader.h"
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

namespace Rizityo::AssetTool
{
	struct LevelData;
	struct Level;
	struct Mesh;
	struct GeometryImportSetting;

	class FBXContext
	{
	public:
		FBXContext(const char* file, Level* level, LevelData* levelData)
			: _Level(level), _LevelData(levelData)
		{
			assert(file && _Level && _LevelData);
			if (InitializeFBX())
			{
				LoadFBXFile(file);
				assert(IsValid());
			}
		}

		~FBXContext()
		{
			_FBXScene->Destroy();
			_FBXManager->Destroy();
			ZeroMemory(this, sizeof(FBXContext));
		}

		void GetLevel(FbxNode* root = nullptr);

		constexpr bool IsValid() const { return _FBXManager && _FBXScene; }
		constexpr float32 LevelScale() const { return _LevelScale; }

	private:
		Level* _Level = nullptr;
		LevelData* _LevelData = nullptr;
		FbxManager* _FBXManager = nullptr;
		FbxScene* _FBXScene = nullptr;
		float32 _LevelScale = 1.0f;

		bool InitializeFBX();
		void LoadFBXFile(const char* file);

		void GetMeshes(FbxNode* node, Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold);
		void GetMesh(FbxNodeAttribute* nodeAttribute, Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold);
		bool GetMeshData(FbxMesh* fbxMesh, Mesh& mesh);
		void GetLODGroup(FbxNodeAttribute* nodeAttribute);
	};
}