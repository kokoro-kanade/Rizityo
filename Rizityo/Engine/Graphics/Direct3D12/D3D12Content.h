#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Content
{
	bool Initialize();
	void Shutdown();

	namespace Submesh
	{
		struct ViewsCache
		{
			D3D12_GPU_VIRTUAL_ADDRESS* const PositionBuffers;
			D3D12_GPU_VIRTUAL_ADDRESS* const ElementBuffers;
			D3D12_INDEX_BUFFER_VIEW* const IndexBufferViews;
			D3D_PRIMITIVE_TOPOLOGY* const PrimitiveTopologies;
			uint32* const ElementsTypes;
		};

		ID::IDType Add(const uint8*& data);
		void Remove(ID::IDType ID);
		void GetViews(const ID::IDType* const gpuIDs, uint32 idCount, OUT const ViewsCache& cache);

	} // Submesh

	namespace Texture
	{
		ID::IDType Add(const uint8* const);
		void Remove(ID::IDType);
		void GetDescriptorIndices(const ID::IDType* const textureIDs, uint32 idCount, OUT uint32* const indices);
	} // Texture

	namespace Material
	{
		struct MaterialsCache
		{
			ID3D12RootSignature** const RootSignatures;
			MaterialType::Type* const MaterialTypes;
		};

		ID::IDType Add(MaterialInitInfo info);
		void Remove(ID::IDType id);
		void GetMaterials(const ID::IDType* const materialIDs, uint32 materialCount, OUT const MaterialsCache& cache);

	} // Material

	namespace RenderItem {

		struct ItemsCache
		{
			ID::IDType* const EntityIDs;
			ID::IDType* const SubmeshGPU_IDs;
			ID::IDType* const MaterialIDs;
			ID3D12PipelineState** const GPassPSOs;
			ID3D12PipelineState** const DepthPSOs;
		};

		ID::IDType Add(ID::IDType entityID, ID::IDType geometryContentID, uint32 materialCount, const ID::IDType* const materialIDs);
		void Remove(ID::IDType id);
		void GetD3D12RenderItemIDs(const FrameInfo& info, OUT Vector<ID::IDType>& d3d12RenderItemIDs);
		void GetItems(const ID::IDType* const d3d12RenderItemIDs, uint32 idCount, OUT const ItemsCache& cache);
	} // RenderItem
}