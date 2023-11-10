#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Rizityo::AssetTool
{
	using namespace Math;
	using namespace DirectX;
	namespace
	{
		using PrimitiveMeshCreateFunc = void(*)(Level&, const PrimitiveInitInfo& info);

		void CreatePlane(Level& level, const PrimitiveInitInfo& info);
		void CreateCube(Level& level, const PrimitiveInitInfo& info);
		void CreateUVSphere(Level& level, const PrimitiveInitInfo& info);
		void CreateIcoSphere(Level& level, const PrimitiveInitInfo& info);
		void CreateCylinder(Level& level, const PrimitiveInitInfo& info);
		void CreateCapsule(Level& level, const PrimitiveInitInfo& info);

		PrimitiveMeshCreateFunc CreateFuncs[]
		{
			CreatePlane,
			CreateCube,
			CreateUVSphere,
			CreateIcoSphere,
			CreateCylinder,
			CreateCapsule
		};

		static_assert(_countof(CreateFuncs) == PrimitiveMeshType::Count);

		struct Axis
		{
			enum : uint32
			{
				x = 0,
				y = 1,
				z = 2
			};
		};

		Mesh CreatePlane(const PrimitiveInitInfo& info,
			uint32 horizontalAxis = Axis::x, uint32 verticalAxis = Axis::z, bool flipWinding = false, 
			Vector3 offset = { -0.5f, 0.f, -0.5f }, Vector2 uRange = { 0.f, 1.f }, Vector2 vRange = { 0.f, 1.f })
		{
			assert(horizontalAxis < 3 && verticalAxis < 3);
			assert(horizontalAxis != verticalAxis);

			const uint32 horizontalCount = Clamp(info.Segments[horizontalAxis], 1u, 10u);
			const uint32 verticalCount = Clamp(info.Segments[verticalAxis], 1u, 10u);
			const float32 horizontalStep = 1.f / horizontalCount;
			const float32 verticalStep = 1.f / verticalCount;
			const float32 uStep = (uRange.y - uRange.x) / horizontalCount;
			const float32 vStep = (vRange.y - vRange.x) / verticalCount;

			Mesh mesh{};
			mesh.Name = "Plane";
			Utility::Vector<Vector2> uvs;

			for (uint32 i = 0; i <= verticalCount; i++)
			{
				for (uint32 j = 0; j <= horizontalCount; j++)
				{
					Vector3 position{ offset };
					float32* const positionAsArray{ &position.x };
					positionAsArray[horizontalAxis] += j * horizontalStep;
					positionAsArray[verticalAxis] += i * verticalStep;
					mesh.Positions.emplace_back(position.x * info.Size.x, position.y * info.Size.y, position.z * info.Size.z);

					//Vector2 uv{ uRange.x, 1.f - vRange.x }; // v�͔��]
					//uv.x += j * uStep;
					//uv.y -= i * vStep;

					Vector2 uv{ 0, 1.f };
					uv.x += (j % 2);
					uv.y -= (i % 2);
					uvs.emplace_back(uv);
				}
			}

			assert(mesh.Positions.size() == (((uint64)horizontalCount + 1) * ((uint64)verticalCount + 1)));

			const uint32 rowLength = horizontalCount + 1;
			for (uint32 i = 0; i < verticalCount; i++)
			{
				uint32 k = 0;
				for (uint32 j = k; j < horizontalCount; j++)
				{
					const uint32 index[4]
					{
						j + i * rowLength,               // ����
						j + (i + 1) * rowLength,		 // ����
						(j + 1) + i * rowLength,		 // �E��
						(j + 1) + (i + 1) * rowLength	 // �E��
					};

					mesh.RawIndices.emplace_back(index[0]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 2 : 1]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 1 : 2]);

					mesh.RawIndices.emplace_back(index[2]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 3 : 1]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 1 : 3]);
				}
				k++;
			}

			const uint32 numIndices = 3 * 2 * horizontalCount * verticalCount;
			assert(mesh.RawIndices.size() == numIndices);

			mesh.UVSets.resize(1);
			for (uint32  i = 0; i < numIndices; i++)
			{
				mesh.UVSets[0].emplace_back(uvs[mesh.RawIndices[i]]);
			}

			return mesh;
		}

		void CreatePlane(Level& level, const PrimitiveInitInfo& info)
		{
			LodGroup lod{};
			lod.Name = "Plane";
			lod.Meshes.emplace_back(CreatePlane(info));
			level.LodGroups.emplace_back(lod);
		}

		void CreateCube(Level& level, const PrimitiveInitInfo& info)
		{

		}

		Mesh CreateUVSphere(const PrimitiveInitInfo& info)
		{
			const uint32 phiCount = Clamp(info.Segments[Axis::x], 3u, 64u);
			const uint32 thetaCount = Clamp(info.Segments[Axis::y], 2u, 64u);
			const float32 phiStep = TWO_PI / phiCount;
			const float32 thetaStep = PI / thetaCount;
			const uint32 numVertices = 2 + phiCount * (thetaCount - 1);
			const uint32 numIndices = 2 * (3 * phiCount) + 2 * 3 * phiCount * (thetaCount - 2);
			
			Mesh mesh{};
			mesh.Name = "UVSphere";
			mesh.Positions.resize(numVertices);

			uint32 c = 0;
			mesh.Positions[c++] = { 0.f, info.Size.y, 0.f };

			for (uint32 i = 1; i <= thetaCount-1; i++)
			{
				const float32 theta = i * thetaStep;
				for (uint32 j = 0; j < phiCount; j++)
				{
					const float32 phi = j * phiStep;
					mesh.Positions[c++] =
					{
						info.Size.x * XMScalarSin(theta) * XMScalarCos(phi),
						info.Size.y * XMScalarCos(theta),
						-info.Size.z * XMScalarSin(theta) * XMScalarSin(phi)
					};
				}
			}

			mesh.Positions[c++] = { 0.f, -info.Size.y, 0.f };

			assert(c == numVertices);

			c = 0;
			mesh.RawIndices.resize(numIndices);
			Utility::Vector<Vector2> uvs{ numIndices };
			const float32 invThetaCount = 1.f / thetaCount;
			const float32 invPhiCount = 1.f / phiCount;

			for (uint32 i = 0; i < phiCount-1; i++)
			{
				uvs[c] = { (2 * i + 1) * 0.5f * invPhiCount, 1.f };
				mesh.RawIndices[c++] = 0;
				uvs[c] = { i * invPhiCount, 1.f - invThetaCount };
				mesh.RawIndices[c++] = i+1;
				uvs[c] = { (i + 1) * invPhiCount, 1.f - invThetaCount };
				mesh.RawIndices[c++] = i+2;
			}

			uvs[c] = { 1.f - 0.5f * invPhiCount, 1.f };
			mesh.RawIndices[c++] = 0;
			uvs[c] = { 1.f - invPhiCount, 1.f - invThetaCount };
			mesh.RawIndices[c++] = phiCount;
			uvs[c] = { 1.f, 1.f - invThetaCount };
			mesh.RawIndices[c++] = 1;

			for (uint32 i = 0; i < thetaCount-2; i++)
			{
				for (uint32 j = 0; j < phiCount-1; j++)
				{
					const uint32 index[4]
					{
						1 + j + i * phiCount,				// ����
						1 + j + (i + 1) * phiCount,			// ����
						1 + (j + 1) + i * phiCount,			// �E��
						1 + (j + 1) + (i + 1) * phiCount	// �E��
					};

					uvs[c] = { j * invPhiCount, 1.f - (i + 1) * invThetaCount };
					mesh.RawIndices[c++] = index[0];
					uvs[c] = { j * invPhiCount, 1.f - (i + 2) * invThetaCount };
					mesh.RawIndices[c++] = index[1];
					uvs[c] = { (j + 1) * invPhiCount, 1.f - (i + 1) * invThetaCount };
					mesh.RawIndices[c++] = index[2];

					uvs[c] = { (j + 1) * invPhiCount, 1.f - (i + 1) * invThetaCount };
					mesh.RawIndices[c++] = index[2];
					uvs[c] = { j * invPhiCount, 1.f - (i + 2) * invThetaCount };
					mesh.RawIndices[c++] = index[1];
					uvs[c] = { (j + 1) * invPhiCount, 1.f - (i + 2) * invThetaCount };
					mesh.RawIndices[c++] = index[3];
				}

				const uint32 index[4]
				{
					phiCount + i * phiCount,
					phiCount + (i + 1) * phiCount,
					1 + i * phiCount,
					1 + (i + 1) * phiCount
				};

				uvs[c] = { 1.f - invPhiCount, 1.f - (i + 1) * invThetaCount };
				mesh.RawIndices[c++] = index[0];
				uvs[c] = { 1.f - invPhiCount, 1.f - (i + 2) * invThetaCount };
				mesh.RawIndices[c++] = index[1];
				uvs[c] = { 1.f, 1.f - (i + 1) * invThetaCount };
				mesh.RawIndices[c++] = index[2];

				uvs[c] = { 1.f, 1.f - (i + 1) * invThetaCount };
				mesh.RawIndices[c++] = index[2];
				uvs[c] = { 1.f - invPhiCount, 1.f - (i + 2) * invThetaCount };
				mesh.RawIndices[c++] = index[1];
				uvs[c] = { 1.f, 1.f - (i + 2) * invThetaCount };
				mesh.RawIndices[c++] = index[3];

			}

			const uint32 southPoleIndex = (uint32)mesh.Positions.size() - 1;
			for (uint32 i = 0; i < phiCount-1; i++)
			{
				uvs[c] = { (2 * i + 1) * 0.5f * invPhiCount, 0.f };
				mesh.RawIndices[c++] = southPoleIndex;
				uvs[c] = { (i + 1) * invPhiCount, invThetaCount };
				mesh.RawIndices[c++] = southPoleIndex - phiCount + i + 1;
				uvs[c] = { i * invPhiCount, invThetaCount };
				mesh.RawIndices[c++] = southPoleIndex - phiCount + i;
			}

			uvs[c] = { 1.f - 0.5f * invPhiCount, 0.f };
			mesh.RawIndices[c++] = southPoleIndex;
			uvs[c] = { 1.f, invThetaCount };
			mesh.RawIndices[c++] = southPoleIndex - phiCount;
			uvs[c] = { 1.f - invPhiCount, invThetaCount };
			mesh.RawIndices[c++] = southPoleIndex - 1;

			assert(c == numIndices);

			mesh.UVSets.emplace_back(uvs);

			return mesh;
		}

		void CreateUVSphere(Level& level, const PrimitiveInitInfo& info)
		{
			LodGroup lod{};
			lod.Name = "UVSphere";
			lod.Meshes.emplace_back(CreateUVSphere(info));
			level.LodGroups.emplace_back(lod);
		}

		void CreateIcoSphere(Level& level, const PrimitiveInitInfo& info)
		{

		}

		void CreateCylinder(Level& level, const PrimitiveInitInfo& info)
		{

		}

		void CreateCapsule(Level& level, const PrimitiveInitInfo& info)
		{

		}

	} // �������

	EDITOR_INTERFACE
	void CreatePrimitiveMesh(LevelData* data, PrimitiveInitInfo* info)
	{
		assert(data && info);
		assert(info->Type < PrimitiveMeshType::Count);
		Level level{};
		CreateFuncs[info->Type](level, *info);

		data->Setting.CalculateNormals = 1;
		ProcessLevel(level, data->Setting);
		PackData(level, *data);
	}
}