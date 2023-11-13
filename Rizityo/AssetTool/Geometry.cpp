#include "Geometry.h"

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
						Vector2& uv1{ mesh.UVSets[0][iRefs[k]] };
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

		void PackVerticesStatic(Mesh& mesh)
		{
			const uint32 numVertices = (uint32)mesh.Vertices.size();
			assert(numVertices);
			mesh.PackedVerticesStatic.reserve(numVertices);

			for (uint32 i = 0; i < numVertices; i++)
			{
				Vertex& v{ mesh.Vertices[i] };
				const uint8 signs = (uint8)((v.Normal.z > 0.f) << 1);
				const uint16 normalX = (uint16)PackFloat<16>(v.Normal.x, -1.f, 1.f);
				const uint16 normalY = (uint16)PackFloat<16>(v.Normal.y, -1.f, 1.f);

				mesh.PackedVerticesStatic.emplace_back(
					PackedVertex::VertexStatic
					{
						v.Position, {0,0,0}, signs,
						{normalX, normalY}, {}, v.UV
					}
				);
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

			PackVerticesStatic(mesh);
		}

		uint64 GetMeshSize(const Mesh& mesh)
		{
			const uint64 numVertices = mesh.Vertices.size();
			const uint64 vertexBufferSize = sizeof(PackedVertex::VertexStatic) * numVertices;
			const uint64 indexSize = (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32);
			const uint64 indexBufferSize = indexSize * mesh.Indices.size();
			constexpr uint64 su32 = sizeof(uint32);
			const uint64 size
			{
				su32 + mesh.Name.size() +   // ���b�V�����̒����ƕ�����f�[�^
				su32 +						// LOD��id
				su32 +						// ���_�T�C�Y
				su32 +						// ���_��
				su32 +						// �C���f�b�N�X�T�C�Y
				su32 +						// �C���f�b�N�X��
				sizeof(float32) +			// LOD threshold
				vertexBufferSize +			// ���_�f�[�^
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

		void PackMeshData(const Mesh& mesh, uint8* const buffer, uint64& at)
		{
			constexpr uint64 su32 = sizeof(uint32);
			uint32 s = 0;

			// ���b�V����
			s = (uint32)mesh.Name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], mesh.Name.c_str(), s); at += s;

			// LOD id
			s = mesh.LOD_ID;
			memcpy(&buffer[at], &s, su32); at += su32;

			// ���_�T�C�Y
			constexpr uint32 vertexSize = sizeof(PackedVertex::VertexStatic);
			s = vertexSize;
			memcpy(&buffer[at], &s, su32); at += su32;

			// ���_��
			const uint32 numVertices = (uint32)mesh.Vertices.size();
			s = numVertices;
			memcpy(&buffer[at], &s, su32); at += su32;

			// �C���f�b�N�X�T�C�Y
			const uint32 indexSize = (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32);
			s = indexSize;
			memcpy(&buffer[at], &s, su32); at += su32;

			// �C���f�b�N�X��
			const uint32 numIndices = (uint32)mesh.Indices.size();
			s = numIndices;
			memcpy(&buffer[at], &s, su32); at += su32;

			// LOD threshold
			memcpy(&buffer[at], &mesh.LODThreshold, sizeof(float32)); at += sizeof(float32);

			// ���_�f�[�^
			s = vertexSize * numVertices;
			memcpy(&buffer[at], mesh.PackedVerticesStatic.data(), s); at += s;

			// �C���f�b�N�X�f�[�^
			s = indexSize * numIndices;
			void* data = (void*)mesh.Indices.data();
			Utility::Vector<uint16> indices;
			// �C���f�b�N�X��32bit�ŕۑ����Ă����̂ŃT�C�Y��16bit�̏ꍇ�͕ϊ�����
			if (indexSize == sizeof(uint16))
			{
				indices.resize(numIndices);
				for (uint32 i = 0; i < numIndices; i++)
				{
					indices[i] = (uint16)mesh.Indices[i];
				}
				data = (void*)indices.data();
			}
			memcpy(&buffer[at], data, s); at += s;
		}

	} // �������

	void ProcessLevel(Level& level, const GeometryImportSetting& setting)
	{
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
		constexpr uint64 su32 = sizeof(uint32);
		const uint64 levelSize = GetLevelSize(level);
		data.DataSize = (uint32)levelSize;
		data.Data = (uint8*)CoTaskMemAlloc(levelSize);
		assert(data.Data);

		uint8* const buffer = data.Data;
		uint64 at = 0;
		uint32 s = 0;

		// ���x�����̒���
		s = (uint32)level.Name.size();
		memcpy(&buffer[at], &s, su32); at += su32;
		// ���x����
		memcpy(&buffer[at], level.Name.c_str(), s); at += s;

		// LOD�̐�
		s = (uint32)level.LODGroups.size();
		memcpy(&buffer[at], &s, su32); at += su32;

		// LOD�f�[�^
		for (auto& lod : level.LODGroups)
		{
			// LOD�̖��O
			s = (uint32)lod.Name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], lod.Name.c_str(), s); at += s;

			// LOD���̃��b�V���̐�
			s = (uint32)lod.Meshes.size();
			memcpy(&buffer[at], &s, su32); at += su32;

			 // ���b�V���f�[�^
			for (auto& mesh : lod.Meshes)
			{
				PackMeshData(mesh, buffer, at);
			}
		}

		assert(levelSize == at);

	}
}