#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Rizityo::AssetTool
{
	using namespace Math;
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
			Utility::Vector<Vector2> uvs;

			for (uint32 i = 0; i < verticalCount; i++)
			{
				for (uint32 j = 0; j < horizontalCount; j++)
				{
					Vector3 position{ offset };
					float32* const positionAsArray{ &position.x };
					positionAsArray[horizontalAxis] += j * horizontalStep;
					positionAsArray[verticalAxis] += i * verticalStep;
					mesh.Positions.emplace_back(position.x * info.Size.x, position.y * info.Size.y, position.z * info.Size.z);

					Vector2 uv{ uRange.x, 1.f - vRange.x }; // v‚Í”½“]
					uv.x += j * uStep;
					uv.y -= i * vStep;
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
						j + i * rowLength,               // ¶ã
						j + (i + 1) * rowLength,		 // ¶‰º
						(j + 1) + i * rowLength,		 // ‰Eã
						(j + 1) + (i + 1) * rowLength	 // ‰E‰º
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

		void CreateCube(Level& level, const PrimitiveInitInfo& info);
		void CreateUVSphere(Level& level, const PrimitiveInitInfo& info);
		void CreateIcoSphere(Level& level, const PrimitiveInitInfo& info);
		void CreateCylinder(Level& level, const PrimitiveInitInfo& info);
		void CreateCapsule(Level& level, const PrimitiveInitInfo& info);
	} // –³–¼‹óŠÔ

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