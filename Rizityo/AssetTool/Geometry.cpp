#include "Geometry.h"
#include "../Utility/IOStream.h"

namespace Rizityo::AssetTool
{
	namespace
	{
		using namespace Math;
		using namespace DirectX;

		void RecalculateNormals(Mesh& mesh)
		{
			const uint32 numIndices = (uint32)mesh.RawIndices.size();
			mesh.Normals.resize(numIndices);

			for (uint32 i = 0; i < numIndices; i++)
			{
				const uint32 i0 = mesh.RawIndices[i];
				const uint32 i1 = mesh.RawIndices[++i];
				const uint32 i2 = mesh.RawIndices[++i];

				XMVECTOR v0{ XMLoadFloat3(&mesh.Positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&mesh.Positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&mesh.Positions[i2]) };

				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };
				XMVECTOR normal{ XMVector3Normalize(XMVector3Cross(e0, e1)) };

				// ��̎O�p�`���̖@���͓���
				XMStoreFloat3(&mesh.Normals[i], normal);
				mesh.Normals[i - 1] = mesh.Normals[i];
				mesh.Normals[i - 2] = mesh.Normals[i];
			}
		}

		void ProcessNormals(Mesh& mesh, float32 smoothingAngle)
		{
			const float32 cosSmoothingAngle = XMScalarCos(PI - smoothingAngle * PI / 180.f);
			const bool isHardEdge = XMScalarNearEqual(smoothingAngle, 180.f, EPSILON);
			const bool isSoftEdge = XMScalarNearEqual(smoothingAngle, 0.f, EPSILON);
			const uint32 numIndices = (uint32)mesh.RawIndices.size();
			const uint32 numVertices = (uint32)mesh.Positions.size();
			assert(numIndices && numVertices);

			mesh.Indices.resize(numIndices);
			Utility::Vector<Utility::Vector<uint32>> indexRefs(numVertices);
			for (uint32 i = 0; i < numIndices; i++)
			{
				indexRefs[mesh.RawIndices[i]].emplace_back(i);
			}

			for (uint32 i = 0; i < numVertices; i++)
			{
				auto& iRefs{ indexRefs[i] };
				uint32 num = (uint32)iRefs.size();
				for (uint32 j = 0; j < num; j++)
				{
					mesh.Indices[iRefs[j]] = (uint32)mesh.Vertices.size(); // ���݂̒��_�ԍ����L�^
					Vertex& v{ mesh.Vertices.emplace_back() }; // �@���̎�ނ̐��������_���ǉ������
					v.Position = mesh.Positions[i];

					// ���_�̖@�����v�Z
					XMVECTOR n1{ XMLoadFloat3(&mesh.Normals[iRefs[j]]) };
					if (!isHardEdge)
					{
						// j�Ԗڂ̖@���Ɠ������݂Ȃ�����̂͑����Ă���폜
						for (uint32 k = j + 1; k < num; k++)
						{
							float32 cosNormal = 0.f;
							XMVECTOR n2{ XMLoadFloat3(&mesh.Normals[iRefs[k]]) };
							if (!isSoftEdge)
							{
								XMStoreFloat(&cosNormal, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (isSoftEdge || cosNormal >= cosSmoothingAngle) // �@���Ԃ̊p�x��smoothAngle�ȉ�
							{
								n1 += n2;
								mesh.Indices[iRefs[k]] = mesh.Indices[iRefs[j]];
								iRefs.erase(iRefs.begin() + k);
								num--;
								k--;
							}
						}
					}
					XMStoreFloat3(&v.Normal, XMVector3Normalize(n1));
				}
			}
		}

		void ProcessUVs(Mesh& mesh)
		{
			Utility::Vector<Vertex> oldVertices;
			oldVertices.swap(mesh.Vertices);
			Utility::Vector<uint32> oldIndices(mesh.Indices.size());
			oldIndices.swap(mesh.Indices);

			const uint32 numVertices = (uint32)oldVertices.size();
			const uint32 numIndices = (uint32)oldIndices.size();
			assert(numVertices && numIndices);

			Utility::Vector<Utility::Vector<uint32>> indexRefs(numVertices);
			for (uint32 i = 0; i < numIndices; i++)
			{
				indexRefs[oldIndices[i]].emplace_back(i);
			}

			for (uint32 i = 0; i < numVertices; i++)
			{
				auto& iRefs = indexRefs[i];
				uint32 num = (uint32)iRefs.size();
				for (uint32 j = 0; j < num; j++)
				{
					mesh.Indices[iRefs[j]] = (uint32)mesh.Vertices.size(); // ���݂̒��_�ԍ����L�^
					Vertex& v{ oldVertices[i] };
					v.UV = mesh.UVSets[0][iRefs[j]];
					mesh.Vertices.emplace_back(v); // UV�̎�ނ̐��������_��ǉ�

					// j�Ԗڂ�UV�Ƃقړ������͍̂폜
					for (uint32 k = j + 1; k < num; k++)
					{
						DX_Vector2& uv1{ mesh.UVSets[0][iRefs[k]] };
						if (XMScalarNearEqual(v.UV.x, uv1.x, EPSILON) && XMScalarNearEqual(v.UV.y, uv1.y, EPSILON))
						{
							mesh.Indices[iRefs[k]] = mesh.Indices[iRefs[j]];
							iRefs.erase(iRefs.begin() + k);
							num--;
							k--;
						}
					}
				}
			}
		}
		
		uint64 GetVertexElementSize(Elements::ElementsType::Type ElementsType)
		{
			using namespace Elements;
			switch (ElementsType)
			{
			case ElementsType::StaticNormal: 
				return sizeof(StaticNormal);
			case ElementsType::StaticNormalTexture:          
				return sizeof(StaticNormalTexture);
			case ElementsType::StaticColor:                   
				return sizeof(StaticColor);
			case ElementsType::Skeletal:                       
				return sizeof(Skeletal);
			case ElementsType::SkeletalColor:                 
				return sizeof(SkeletalColor);
			case ElementsType::SkeletalNormal:                
				return sizeof(SkeletalNormal);
			case ElementsType::SkeletalNormalColor:          
				return sizeof(SkeletalNormalColor);
			case ElementsType::SkeletalNormalTexture:        
				return sizeof(SkeletalNormalTexture);
			case ElementsType::SkeletalNormalTextureColor:  
				return sizeof(SkeletalNormalTextureColor);
			}

			return 0;
		}

		void DetermineElementsType(Mesh& mesh)
		{
			using namespace Elements;
			if (mesh.Normals.size())
			{
				if (mesh.UVSets.size() && mesh.UVSets[0].size())
				{
					mesh.ElementsType = ElementsType::StaticNormalTexture;
				}
				else
				{
					mesh.ElementsType = ElementsType::StaticNormal;
				}
			}
			else if (mesh.Colors.size())
			{
				mesh.ElementsType = ElementsType::StaticColor;
			}

			// TODO: �X�P���^�����b�V��
		}

		void PackVertices(Mesh& mesh)
		{
			const uint32 numVertices = (uint32)mesh.Vertices.size();
			assert(numVertices);

			mesh.PositionBuffer.resize(sizeof(Math::DX_Vector3) * numVertices);
			Math::DX_Vector3* const positionBuffer = (Math::DX_Vector3* const)mesh.PositionBuffer.data();

			// ���_�ʒu�̊i�[
			for (uint32 i = 0; i < numVertices; i++)
			{
				positionBuffer[i] = mesh.Vertices[i].Position;
			}

			struct u16v2 { uint16 x, y; };
			struct u8v3 { uint8 x, y, z; };

			Utility::Vector<uint8> TSigns(numVertices);
			Utility::Vector<u16v2> Normals(numVertices);
			Utility::Vector<u16v2> Tangents(numVertices);
			Utility::Vector<u8v3> JointWeights(numVertices);

			if (mesh.ElementsType & Elements::ElementsType::StaticNormal)
			{
				// �@��
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					TSigns[i] = (uint8)((v.Normal.z > 0.f) << 1);
					Normals[i] = { (uint16)PackFloat<16>(v.Normal.x, -1.f, 1.f), (uint16)PackFloat<16>(v.Normal.y, -1.f, 1.f) };
				}

				// �ڐ�
				if (mesh.ElementsType & Elements::ElementsType::StaticNormalTexture)
				{
					for (uint32 i = 0; i < numVertices; i++)
					{
						Vertex& v{ mesh.Vertices[i] };
						TSigns[i] |= (uint8)((v.Tangent.w > 0.f) && (v.Tangent.z > 0.f));
						Tangents[i] = { (uint16)PackFloat<16>(v.Tangent.x, -1.f, 1.f), (uint16)PackFloat<16>(v.Tangent.y, -1.f, 1.f) };
					}
				}
			}

			if (mesh.ElementsType & Elements::ElementsType::Skeletal)
			{
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					// �W���C���g�̏d�� ([0.0, 1.0]����[0..255]�ɕϊ�)
					JointWeights[i] = {
						(uint8)PackUnitFloat<8>(v.JointWeights.x),
						(uint8)PackUnitFloat<8>(v.JointWeights.y),
						(uint8)PackUnitFloat<8>(v.JointWeights.z) };

					// �W���C���g�̏d�݂̑��a��1�Ȃ̂Ŏl�ڂ̐����͕K�v�Ȃ�
				}
			}

			mesh.ElementBuffer.resize(GetVertexElementSize(mesh.ElementsType) * numVertices);
			using namespace Elements;

			// ���_�̑����ɉ����ėv�f���i�[
			switch (mesh.ElementsType)
			{
			case ElementsType::StaticColor:
			{
				StaticColor* const elementBuffer{ (StaticColor* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					elementBuffer[i] = { {v.Red, v.Green, v.Blue}, {} };
				}
			}
			break;
			case ElementsType::StaticNormal:
			{
				StaticNormal* const elementBuffer{ (StaticNormal* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					elementBuffer[i] = { {v.Red, v.Green, v.Blue}, TSigns[i], {Normals[i].x, Normals[i].y} };
				}
			}
			break;
			case ElementsType::StaticNormalTexture:
			{
				StaticNormalTexture* const elementBuffer{ (StaticNormalTexture* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					elementBuffer[i] = { {v.Red, v.Green, v.Blue}, TSigns[i],
										 {Normals[i].x, Normals[i].y}, {Tangents[i].x, Tangents[i].y},
										 v.UV };
				}
			}
			break;
			case ElementsType::Skeletal:
			{
				Skeletal* const elementBuffer{ (Skeletal* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, {},
										 {indices[0], indices[1], indices[2], indices[3]} };
				}
			}
			break;
			case ElementsType::SkeletalColor:
			{
				SkeletalColor* const elementBuffer{ (SkeletalColor* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, {},
										 {indices[0], indices[1], indices[2], indices[3]},
										 {v.Red, v.Green, v.Blue}, {} };
				}
			}
			break;
			case ElementsType::SkeletalNormal:
			{
				SkeletalNormal* const elementBuffer{ (SkeletalNormal* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, TSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {Normals[i].x, Normals[i].y} };
				}
			}
			break;
			case ElementsType::SkeletalNormalColor:
			{
				SkeletalNormalColor* const elementBuffer{ (SkeletalNormalColor* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, TSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {Normals[i].x, Normals[i].y}, {v.Red, v.Green, v.Blue}, {} };
				}
			}
			break;
			case ElementsType::SkeletalNormalTexture:
			{
				SkeletalNormalTexture* const elementBuffer{ (SkeletalNormalTexture* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, TSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {Normals[i].x, Normals[i].y}, {Tangents[i].x, Tangents[i].y}, v.UV };
				}
			}
			break;
			case ElementsType::SkeletalNormalTextureColor:
			{
				SkeletalNormalTextureColor* const elementBuffer{ (SkeletalNormalTextureColor* const)mesh.ElementBuffer.data() };
				for (uint32 i = 0; i < numVertices; i++)
				{
					Vertex& v{ mesh.Vertices[i] };
					const uint16 indices[4]{ (uint16)v.JointIndices.x, (uint16)v.JointIndices.y , (uint16)v.JointIndices.z , (uint16)v.JointIndices.w };
					elementBuffer[i] = { {JointWeights[i].x, JointWeights[i].y, JointWeights[i].z}, TSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {Normals[i].x, Normals[i].y}, {Tangents[i].x, Tangents[i].y}, v.UV,
										 {v.Red, v.Green, v.Blue}, {} };
				}
			}
			break;
			}
		}

		void ProcessVertices(Mesh& mesh, const GeometryImportSetting& setting)
		{
			assert((mesh.RawIndices.size() % 3) == 0);
			if (setting.CalculateNormals || mesh.Normals.empty())
			{
				RecalculateNormals(mesh);
			}

			ProcessNormals(mesh, setting.SmoothingAngle);

			if (!mesh.UVSets.empty())
			{
				ProcessUVs(mesh);
			}

			DetermineElementsType(mesh);
			PackVertices(mesh);
		}

		uint64 GetMeshSize(const Mesh& mesh)
		{
			const uint64 numVertices = mesh.Vertices.size();
			const uint64 positionBufferSize{ mesh.PositionBuffer.size() };
			assert(positionBufferSize == sizeof(Math::DX_Vector3) * numVertices);
			const uint64 elementBufferSize{ mesh.ElementBuffer.size() };
			assert(elementBufferSize == GetVertexElementSize(mesh.ElementsType) * numVertices);
			const uint64 indexSize = (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32);
			const uint64 indexBufferSize = indexSize * mesh.Indices.size();
			constexpr uint64 su32 = sizeof(uint32);
			const uint64 size
			{
				su32 + mesh.Name.size() +   // ���b�V�����̒����ƕ�����f�[�^
				su32 +						// LOD��id
				su32 +						// ���_�����T�C�Y(�ʒu�͏���)
				su32 +						// ���_����
				su32 +						// ���_��
				su32 +						// �C���f�b�N�X�T�C�Y
				su32 +						// �C���f�b�N�X��
				sizeof(float32) +			// LOD threshold
				positionBufferSize +		// ���_�ʒu�f�[�^
				elementBufferSize +		// ���_�����f�[�^
				indexBufferSize				// �C���f�b�N�X�f�[�^
			};

			return size;
		}

		uint64 GetLevelSize(const Level& level)
		{
			constexpr uint64 su32 = sizeof(uint32);
			uint64 size{
				su32 + level.Name.size() +	 // ���x�����ƕ�����f�[�^
				su32						 // LOD�̐�
			};

			for (auto& lod : level.LODGroups)
			{
				uint64 lodSize
				{
					su32 + lod.Name.size() + // LOD���ƕ�����f�[�^
					su32					 // LOD���̃��b�V���̐�
				};

				// ���b�V���f�[�^
				for (auto& mesh : lod.Meshes)
				{
					lodSize += GetMeshSize(mesh);
				}

				size += lodSize;
			}

			return size;
		}

		void PackMeshData(const Mesh& mesh,Utility::BinaryWriter& binaryWriter)
		{
			// ���b�V����
			binaryWriter.Write((uint32)mesh.Name.size());
			binaryWriter.Write(mesh.Name.c_str(), mesh.Name.size());

			// LOD ID
			binaryWriter.Write(mesh.LOD_ID);

			// ���_�����T�C�Y
			const uint32 elementsSize{ (uint32)GetVertexElementSize(mesh.ElementsType) };
			binaryWriter.Write(elementsSize);

			// ���_����
			binaryWriter.Write((uint32)mesh.ElementsType);

			// ���_��
			const uint32 numVertices = (uint32)mesh.Vertices.size();
			binaryWriter.Write(numVertices);

			// �C���f�b�N�X�T�C�Y
			const uint32 indexSize = (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32);
			binaryWriter.Write(indexSize);

			// �C���f�b�N�X��
			const uint32 numIndices = (uint32)mesh.Indices.size();
			binaryWriter.Write(numIndices);

			// LOD threshold
			binaryWriter.Write(mesh.LODThreshold);

			// ���_�ʒu�f�[�^
			assert(mesh.PositionBuffer.size() == sizeof(Math::DX_Vector3) * numVertices);
			binaryWriter.Write(mesh.PositionBuffer.data(), mesh.PositionBuffer.size());

			// ���_�����f�[�^
			assert(mesh.ElementBuffer.size() == elementsSize * numVertices);
			binaryWriter.Write(mesh.ElementBuffer.data(), mesh.ElementBuffer.size());

			// �C���f�b�N�X�f�[�^
			const uint32 index_buffer_size{ indexSize * numIndices };
			const uint8* data{ (const uint8*)mesh.Indices.data() };
			Utility::Vector<uint16> indices;
			// �C���f�b�N�X��32bit�ŕۑ����Ă����̂ŃT�C�Y��16bit�̏ꍇ�͕ϊ�����
			if (indexSize == sizeof(uint16))
			{
				indices.resize(numIndices);
				for (uint32 i = 0; i < numIndices; i++)
				{
					indices[i] = (uint16)mesh.Indices[i];
				}
				data = (const uint8*)indices.data();
			}
			binaryWriter.Write(data, index_buffer_size);
		}

		bool SplitMeshByMaterial(uint32 materialIndex, const Mesh& mesh, Mesh& submesh)
		{
			submesh.Name = mesh.Name;
			submesh.LODThreshold = mesh.LODThreshold;
			submesh.LOD_ID = mesh.LOD_ID;
			submesh.MaterialUsed.emplace_back(materialIndex);
			submesh.UVSets.resize(mesh.UVSets.size());

			const uint32 numPolygons = static_cast<uint32>(mesh.RawIndices.size()) / 3;
			Utility::Vector<uint32> vertexRef(mesh.Positions.size(), UINT32_INVALID_NUM);

			for (uint32 i = 0; i < numPolygons; i++)
			{
				const uint32 mIndex = mesh.MaterialIndices[i];
				if (mIndex != materialIndex)
					continue;

				const uint32 index = i * 3;
				for (uint32 j = index; j < index + 3; j++)
				{
					const uint32 vIndex = mesh.RawIndices[j];
					if (vertexRef[vIndex] != UINT32_INVALID_NUM)
					{
						submesh.RawIndices.emplace_back(vertexRef[vIndex]);
					}
					else
					{
						submesh.RawIndices.emplace_back(static_cast<uint32>(submesh.Positions.size()));
						vertexRef[vIndex] = submesh.RawIndices.back();
						submesh.Positions.emplace_back(mesh.Positions[vIndex]);
					}

					if (mesh.Normals.size())
					{
						submesh.Normals.emplace_back(mesh.Normals[j]);
					}

					if (mesh.Tangents.size())
					{
						submesh.Tangents.emplace_back(mesh.Tangents[j]);
					}

					for (uint32 k = 0; k < mesh.UVSets.size(); k++)
					{
						if (mesh.UVSets[k].size())
						{
							submesh.UVSets[k].emplace_back(mesh.UVSets[k][j]);
						}
					}
				}
			}

			assert((submesh.RawIndices.size() % 3) == 0);
			return !submesh.RawIndices.empty();
		}

		void SplitMeshesByMaterial(Level& level)
		{
			for (auto& lodGroup : level.LODGroups)
			{
				Utility::Vector<Mesh> newMeshes;

				for (auto& mesh : lodGroup.Meshes)
				{
					// ��ȏ�}�e���A�����g���Ă���΃T�u���b�V���ɕ�������
					const uint32 numMaterials = static_cast<uint32>(mesh.MaterialUsed.size());
					if (numMaterials > 1)
					{
						for (uint32 i = 0; i < numMaterials; i++)
						{
							Mesh submesh{};
							if (SplitMeshByMaterial(mesh.MaterialUsed[i], mesh, submesh))
							{
								newMeshes.emplace_back(submesh);
							}
						}
					}
					else
					{
						newMeshes.emplace_back(mesh);
					}
				}

				newMeshes.swap(lodGroup.Meshes);
			}
		}

	} // �������

	void ProcessLevel(Level& level, const GeometryImportSetting& setting)
	{
		SplitMeshesByMaterial(level);

		for (auto& lod : level.LODGroups)
		{
			for (auto& mesh : lod.Meshes)
			{
				ProcessVertices(mesh, setting);
			}
		}
	}

	void PackData(const Level& level, LevelData& data)
	{
		const uint64 levelSize = GetLevelSize(level);
		data.DataSize = (uint32)levelSize;
		data.Data = (uint8*)CoTaskMemAlloc(levelSize);
		assert(data.Data);

		Utility::BinaryWriter binaryWriter{ data.Data, data.DataSize };

		// ���x�����̒���
		binaryWriter.Write((uint32)level.Name.size());

		// ���x����
		binaryWriter.Write(level.Name.c_str(), level.Name.size());

		// LOD�O���[�v�̐�
		binaryWriter.Write((uint32)level.LODGroups.size());

		// LOD�O���[�v�f�[�^
		for (auto& lodGroup : level.LODGroups)
		{
			// LOD�O���[�v���̒���
			binaryWriter.Write((uint32)lodGroup.Name.size());

			// LOD�O���[�v��
			binaryWriter.Write(lodGroup.Name.c_str(), lodGroup.Name.size());

			// LOD�O���[�v���̃��b�V���̐�
			binaryWriter.Write((uint32)lodGroup.Meshes.size());

			// ���b�V���f�[�^
			for (auto& mesh : lodGroup.Meshes)
			{
				PackMeshData(mesh, binaryWriter);
			}
		}

		assert(levelSize == binaryWriter.offset());

	}
}