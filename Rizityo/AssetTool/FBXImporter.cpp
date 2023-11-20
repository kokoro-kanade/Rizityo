#include "FBXImporter.h"
#include "Geometry.h"

#if _DEBUG
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\debug\\libfbxsdk-md.lib")
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\debug\\libxml2-md.lib")
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\debug\\zlib-md.lib")
#else
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\release\\libfbxsdk-md.lib")
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\release\\libxml2-md.lib")
#pragma comment (lib,"C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.4\\lib\\vs2019\\x64\\release\\zlib-md.lib")
#endif // _DEBUG


namespace Rizityo::AssetTool
{
	namespace
	{
		std::mutex FBXMutex{};
	} // 無名空間

	bool FBXContext::InitializeFBX()
	{
		assert(!IsValid());

		_FBXManager = FbxManager::Create();
		if (!_FBXManager)
		{
			return false;
		}

		FbxIOSettings* ioSetting{ FbxIOSettings::Create(_FBXManager, IOSROOT) };
		assert(ioSetting);
		_FBXManager->SetIOSettings(ioSetting);

		return true;
	}

	void FBXContext::LoadFBXFile(const char* file)
	{
		assert(_FBXManager && !_FBXScene);
		_FBXScene = FbxScene::Create(_FBXManager, "Importer Scene");
		if (!_FBXScene)
			return;

		FbxImporter* importer = FbxImporter::Create(_FBXManager, "Importer");
		/*if (!(importer && importer->Initialize(file, -1, _FBXManager->GetIOSettings()) && importer->Import(_FBXScene)))
			return;*/

		if (!importer)
		{
			return;
		}

		// 注意: パスに日本語が含まれているとダメっぽい
		bool result = importer->Initialize(file, -1, _FBXManager->GetIOSettings());
		if (!result)
		{
			std::string error = importer->GetStatus().GetErrorString();
			FbxStatus status = importer->GetStatus().GetCode();
			return;
		}

		if (!importer->Import(_FBXScene))
		{
			return;
		}

		importer->Destroy();
		_LevelScale = static_cast<float32>(_FBXScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m));
	}

	void FBXContext::GetLevel(FbxNode* root /*= nullptr*/)
	{
		assert(IsValid());

		if (!root)
		{
			root = _FBXScene->GetRootNode();
			if (!root)
				return;
		}

		const int32 numNodes = root->GetChildCount();
		for (int32 i = 0; i < numNodes; i++)
		{
			FbxNode* node = root->GetChild(i);
			if (!node)
				continue;

			LODGroup lodGroup{};
			GetMeshes(node, lodGroup.Meshes, 0, -1.f);
			if (lodGroup.Meshes.size())
			{
				lodGroup.Name = lodGroup.Meshes[0].Name;
				_Level->LODGroups.emplace_back(lodGroup);
			}
		}
	}

	void FBXContext::GetMeshes(FbxNode* node, Utility::Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold)
	{
		assert(node && lodID != UINT32_INVALID_NUM);

		bool isLODGroup = false;
		if (const int32 numAttributes = node->GetNodeAttributeCount())
		{
			for (int32 i = 0; i < numAttributes; i++)
			{
				FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(i);
				const FbxNodeAttribute::EType attributeType{ attribute->GetAttributeType() };
				if (attributeType == FbxNodeAttribute::eMesh)
				{
					GetMesh(attribute, meshes, lodID, lodThreshold);
				}
				else if (attributeType == FbxNodeAttribute::eLODGroup)
				{
					GetLODGroup(attribute);
					isLODGroup = true;
				}
			}
		}

		if (!isLODGroup)
		{
			if (const int32 numChildren = node->GetChildCount())
			{
				for (int32 i = 0; i < numChildren; i++)
				{
					GetMeshes(node->GetChild(i), meshes, lodID, lodThreshold);
				}
			}
		}
	}

	void FBXContext::GetMesh(FbxNodeAttribute* nodeAttribute, Utility::Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold)
	{
		assert(nodeAttribute);

		FbxMesh* fbxMesh = (FbxMesh*)nodeAttribute ;
		if (fbxMesh->RemoveBadPolygons() < 0)
			return;

		// 必要なら三角形分割
		FbxGeometryConverter gc{ _FBXManager };
		fbxMesh = (FbxMesh*)gc.Triangulate(fbxMesh, true);
		if (!fbxMesh || fbxMesh->RemoveBadPolygons() < 0)
			return;

		FbxNode* const node = fbxMesh->GetNode();

		Mesh mesh;
		mesh.LOD_ID = lodID;
		mesh.LODThreshold = lodThreshold;
		mesh.Name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

		if (GetMeshData(fbxMesh, mesh))
		{
			meshes.emplace_back(mesh);
		}
	}

	bool FBXContext::GetMeshData(FbxMesh* fbxMesh, Mesh& mesh)
	{
		assert(fbxMesh);

		FbxNode* const node{ fbxMesh->GetNode() };
		FbxAMatrix geometricTransform;

		geometricTransform.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
		geometricTransform.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
		geometricTransform.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));

		FbxAMatrix transform{ node->EvaluateGlobalTransform() * geometricTransform };
		FbxAMatrix inverse_transpose{ transform.Inverse().Transpose() };

		const int32 numPolygons = fbxMesh->GetPolygonCount();
		if (numPolygons <= 0)
			return false;

		// 頂点の取得
		const int32 numVertices = fbxMesh->GetControlPointsCount();
		FbxVector4* vertices = fbxMesh->GetControlPoints();
		const int32 numIndices = fbxMesh->GetPolygonVertexCount();
		int32* indices = fbxMesh->GetPolygonVertices();

		assert(numVertices > 0 && vertices && numIndices > 0 && indices);
		if (!(numVertices > 0 && vertices && numIndices > 0 && indices))
			return false;

		mesh.RawIndices.resize(numIndices);
		Utility::Vector vertexRef(numVertices, UINT32_INVALID_NUM);

		for (int32 i = 0; i < numIndices; i++)
		{
			const uint32 vIndex = static_cast<uint32>(indices[i]);
			if (vertexRef[vIndex] != UINT32_INVALID_NUM)
			{
				mesh.RawIndices[i] = vertexRef[vIndex];
			}
			else
			{
				FbxVector4 v = transform.MultT(vertices[vIndex]) * _LevelScale;
				mesh.RawIndices[i] = static_cast<uint32>(mesh.Positions.size());
				vertexRef[vIndex] = mesh.RawIndices[i];
				mesh.Positions.emplace_back(static_cast<float32>(v[0]), static_cast<float32>(v[1]), static_cast<float32>(v[2]));
			}
		}

		assert(mesh.RawIndices.size() % 3 == 0);

		// ポリゴンごとのマテリアルのインデックスの取得
		assert(numPolygons > 0);
		FbxLayerElementArrayTemplate<int32>* materialIndices;
		if (fbxMesh->GetMaterialIndices(&materialIndices))
		{
			for (int32 i = 0; i < numPolygons; i++)
			{
				const int32 materialIndex = materialIndices->GetAt(i);
				assert(materialIndex >= 0);
				mesh.MaterialIndices.emplace_back(static_cast<uint32>(materialIndex));
				if (std::find(mesh.MaterialUsed.begin(), mesh.MaterialUsed.end(), static_cast<uint32>(materialIndex)) == mesh.MaterialUsed.end())
				{
					mesh.MaterialUsed.emplace_back(static_cast<uint32>(materialIndex));
				}
			}
		}

		// デフォルトでは法線のインポート設定はON
		const bool importNormals = (!_LevelData->Setting.CalculateNormals);
		// デフォルトでは接線のインポート設定はON
		const bool importTangents = (!_LevelData->Setting.CalculateTangents);

		// 法線のインポート
		if (importNormals)
		{
			FbxArray<FbxVector4> normals;
			if (fbxMesh->GenerateNormals() && fbxMesh->GetPolygonVertexNormals(normals) && normals.Size() > 0)
			{
				const int32 numNormals = normals.Size();
				for (int32 i = 0; i < numNormals; i++)
				{
					FbxVector4 normal{ inverse_transpose.MultT(normals[i]) };
					normal.Normalize();
					mesh.Normals.emplace_back(static_cast<float32>(normal[0]), static_cast<float32>(normal[1]), static_cast<float32>(normal[2]));
				}
			}
			else
			{
				_LevelData->Setting.CalculateNormals = true;
			}
		}

		// 接線のインポート
		if (importTangents)
		{
			FbxLayerElementArrayTemplate<FbxVector4>* tangents = nullptr;
			if (fbxMesh->GenerateTangentsData() && fbxMesh->GetTangents(&tangents) && tangents && tangents->GetCount() > 0)
			{
				const int32 numTangent = tangents->GetCount();
				for (int32 i = 0; i < numTangent; i++)
				{
					FbxVector4 tangent = tangents->GetAt(i);
					const float32 handedness = (float32)tangent[3]; // 接線の第４成分に右手/左手座標系の情報が入っている
					tangent[3] = 0.0;
					tangent.Normalize();
					tangent = inverse_transpose.MultT(tangent);
					mesh.Tangents.emplace_back(static_cast<float32>(tangent[0]), static_cast<float32>(tangent[1]), static_cast<float32>(tangent[2]), handedness);
				}
			}
			else
			{
				_LevelData->Setting.CalculateTangents = true;
			}
		}

		// UVの取得
		FbxStringList uvNames;
		fbxMesh->GetUVSetNames(uvNames);
		const int32 uvSetCount = uvNames.GetCount();

		mesh.UVSets.resize(uvSetCount);

		for (int32 i = 0; i < uvSetCount; i++)
		{
			FbxArray<FbxVector2> uvs;
			if (fbxMesh->GetPolygonVertexUVs(uvNames.GetStringAt(i), uvs))
			{
				const int32 numUVs = uvs.Size();
				for (int32 j = 0; j < numUVs; j++)
				{
					mesh.UVSets[i].emplace_back(static_cast<float32>(uvs[j][0]), static_cast<float32>(uvs[j][1]));
				}
			}
		}

		return true;
	}

	void FBXContext::GetLODGroup(FbxNodeAttribute* nodeAttribute)
	{
		assert(nodeAttribute);

		FbxLODGroup* fbxLODGroup{ (FbxLODGroup*)nodeAttribute };
		FbxNode* const node = fbxLODGroup->GetNode();
		LODGroup lodGroup{};
		lodGroup.Name = (node->GetName()[0] != '\0') ? node->GetName() : fbxLODGroup->GetName();

		const int32 numNodes = node->GetChildCount();
		assert(numNodes > 0 && fbxLODGroup->GetNumThresholds() == (numNodes - 1));

		for (int32 i = 0; i < numNodes; i++)
		{
			float32 lodThreshold = -1.f;
			if (i > 0)
			{
				FbxDistance threshold;
				fbxLODGroup->GetThreshold(i - 1, threshold);
				lodThreshold = threshold.value() * _LevelScale;
			}

			GetMeshes(node->GetChild(i), lodGroup.Meshes, (uint32)lodGroup.Meshes.size(), lodThreshold);
		}

		if (lodGroup.Meshes.size()) {
			_Level->LODGroups.emplace_back(lodGroup);
		}
	}

	EDITOR_INTERFACE
	void ImportFBX(const char* filePath, LevelData* levelData)
	{
		assert(filePath && levelData);
		Level level{};

		{
			std::lock_guard lock{ FBXMutex };
			FBXContext fbxContext{ filePath, &level, levelData };
			if (fbxContext.IsValid())
			{
				fbxContext.GetLevel();
			}
			else
			{
				// TODO: send failure log message to editor
				return;
			}
		}

		ProcessLevel(level, levelData->Setting);
		PackData(level, *levelData);
	}
}