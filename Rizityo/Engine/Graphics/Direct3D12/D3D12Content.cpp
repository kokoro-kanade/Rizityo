#include "D3D12Content.h"
#include "D3D12Core.h"
#include "D3D12GeometryPass.h"
#include "Utility/IOStream.h"
#include "Content/ContentToEngine.h"

namespace Rizityo::Graphics::D3D12::Content
{
	namespace
	{
		struct SubmeshView
		{
			D3D12_VERTEX_BUFFER_VIEW PositionBufferView{};
			D3D12_VERTEX_BUFFER_VIEW ElementBufferView{};
			D3D12_INDEX_BUFFER_VIEW IndexBufferView{};
			D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;
			uint32 ElementsType{};
		};

		Utility::FreeList<ID3D12Resource*> SubmeshBuffers{};
		Utility::FreeList<SubmeshView> SubmeshViews{};
		std::mutex SubmeshMutex{};

		Utility::FreeList<D3D12Texture> Textures;
		std::mutex TextureMutex{};

		Utility::Vector<ID3D12RootSignature*> RootSignatures;
		std::unordered_map<uint64, ID::IDType> MaterialRootSignatureMap; // マテリアルタイプ・シェーダーフラグからルートシグネチャーへのマップ
		Utility::FreeList<std::unique_ptr<uint8[]>> Materials;
		std::mutex MaterialMutex{};

		ID::IDType CreateRootSignature(MaterialType::Type type, ShaderFlags::Flags flags);

		class D3D12MaterialStream
		{
		public:

			DISABLE_COPY_AND_MOVE(D3D12MaterialStream);

			explicit D3D12MaterialStream(uint8* const materialBuffer)
				:_Buffer{ materialBuffer }
			{
				Initialize();
			}

			explicit D3D12MaterialStream(OUT std::unique_ptr<uint8[]>& materialBuffer, MaterialInitInfo info)
			{
				assert(!materialBuffer);

				uint32 shaderCount = 0;
				uint32 flags = 0;
				for (uint32 i = 0; i < ShaderType::Count; i++)
				{
					if (ID::IsValid(info.ShaderIDs[i]))
					{
						shaderCount++;
						flags |= (1 << i);
					}
				}

				assert(shaderCount && flags);

				const uint32 bufferSize{
					sizeof(MaterialType::Type) +
					sizeof(ShaderFlags::Flags) +
					sizeof(ID::IDType) +                                      // ルートシグネチャーID
					sizeof(uint32) +                                          // テクスチャ数
					sizeof(ID::IDType) * shaderCount +                        // シェーダーIDs
					(sizeof(ID::IDType) + sizeof(uint32)) * info.TextureCount // テクスチャIDs, Descriptor Indices (テクスチャを使わない場合は0)
				};

				materialBuffer = std::make_unique<uint8[]>(bufferSize);
				_Buffer = materialBuffer.get();
				uint8* const buffer{ _Buffer };

				*(MaterialType::Type*)buffer = info.Type;
				*(ShaderFlags::Flags*)(&buffer[ShaderFlagsIndex]) = (ShaderFlags::Flags)flags;
				*(ID::IDType*)(&buffer[RootSignatureIndex]) = CreateRootSignature(info.Type, (ShaderFlags::Flags)flags);
				*(uint32*)(&buffer[TextureCountIndex]) = info.TextureCount;

				Initialize();

				if (info.TextureCount)
				{
					memcpy(_TextureIDs, info.TextureIDs, info.TextureCount * sizeof(ID::IDType));
					Texture::GetDescriptorIndices(_TextureIDs, info.TextureCount, _DescriptorIndices);
				}

				uint32 shaderIndex = 0;
				for (uint32 i = 0; i < ShaderType::Count; i++)
				{
					if (ID::IsValid(info.ShaderIDs[i]))
					{
						_ShaderIDs[shaderIndex] = info.ShaderIDs[i];
						++shaderIndex;
					}
				}

				assert(shaderIndex == (uint32)_mm_popcnt_u32(_ShaderFlags));
			}

			[[nodiscard]] constexpr uint32 TextureCount() const { return _TextureCount; }
			[[nodiscard]] constexpr MaterialType::Type MaterialType() const { return _Type; }
			[[nodiscard]] constexpr ShaderFlags::Flags ShaderFlags() const { return _ShaderFlags; }
			[[nodiscard]] constexpr ID::IDType RootSignatureID() const { return _RootSignatureID; }
			[[nodiscard]] constexpr ID::IDType* TextureIDs() const { return _TextureIDs; }
			[[nodiscard]] constexpr uint32* DectriptorIndices() const { return _DescriptorIndices; }
			[[nodiscard]] constexpr ID::IDType* ShaderIDs() const { return _ShaderIDs; }

		private:
			void Initialize()
			{
				assert(_Buffer);
				uint8* const buffer{ _Buffer };

				_Type = *(MaterialType::Type*)buffer;
				_ShaderFlags = *(ShaderFlags::Flags*)(&buffer[ShaderFlagsIndex]);
				_RootSignatureID = *(ID::IDType*)(&buffer[RootSignatureIndex]);
				_TextureCount = *(uint32*)(&buffer[TextureCountIndex]);

				_ShaderIDs = (ID::IDType*)(&buffer[TextureCountIndex + sizeof(uint32)]);
				_TextureIDs = _TextureCount ? &_ShaderIDs[_mm_popcnt_u32(_ShaderFlags)] : nullptr;
				_DescriptorIndices = _TextureCount ? (uint32*)(&_TextureIDs[_TextureCount]) : nullptr;
			}

			constexpr static uint32 ShaderFlagsIndex{ sizeof(MaterialType::Type) };
			constexpr static uint32 RootSignatureIndex{ ShaderFlagsIndex + sizeof(ShaderFlags::Flags) };
			constexpr static uint32 TextureCountIndex{ RootSignatureIndex + sizeof(ID::IDType) };

			uint8* _Buffer;
			ID::IDType* _TextureIDs;
			uint32* _DescriptorIndices;
			ID::IDType* _ShaderIDs;
			ID::IDType _RootSignatureID;
			uint32 _TextureCount;
			MaterialType::Type _Type;
			ShaderFlags::Flags _ShaderFlags;
		};

		struct D3D12RenderItem
		{
			ID::IDType EntityID;
			ID::IDType SubmeshGPU_ID;
			ID::IDType MaterialID;
			ID::IDType PSO_ID;
			ID::IDType DepthPSO_ID;
		};

		Utility::FreeList<D3D12RenderItem> RenderItems;
		Utility::FreeList<std::unique_ptr<ID::IDType[]>> RenderItemIDs;
		std::mutex RenderItemMutex{};

		struct PSO_ID
		{
			ID::IDType GPassPSO_ID{ ID::INVALID_ID };
			ID::IDType DepthPSO_ID{ ID::INVALID_ID };
		};

		Utility::Vector<ID3D12PipelineState*> PipelineStates;
		std::unordered_map<uint64, ID::IDType> PSO_Map;
		std::mutex PSO_Mutex{};

		struct {
			Utility::Vector<Rizityo::Content::LOD_Offset> LOD_Offsets;
			Utility::Vector<ID::IDType> GeometryIDs;
		} FrameCache;

	} // 変数

	namespace
	{
		constexpr D3D_PRIMITIVE_TOPOLOGY GetD3D_PrimitiveTopology(PrimitiveTopology::Type type)
		{
			assert(type < PrimitiveTopology::Count);

			switch (type)
			{
			case PrimitiveTopology::PointList:     return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleList:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			}

			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D_PrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY topology)
		{
			switch (topology)
			{
			case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
			case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			}

			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}

		constexpr D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureFlags(ShaderFlags::Flags flags)
		{
			D3D12_ROOT_SIGNATURE_FLAGS defaultFlags{ Helper::D3D12RootSignatureDesc::DefaultFlags };
			if (flags & ShaderFlags::Vertex)           defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Hull)             defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Domain)           defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Geometry)         defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Pixel)            defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Amplification)    defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
			if (flags & ShaderFlags::Mesh)             defaultFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
			return defaultFlags;
		}

		ID::IDType CreateRootSignature(MaterialType::Type type, ShaderFlags::Flags flags)
		{
			assert(type < MaterialType::Count);
			static_assert(sizeof(type) == sizeof(uint32) && sizeof(flags) == sizeof(uint32));
			const uint64 key = ((uint64)type << 32) | flags;
			auto pair = MaterialRootSignatureMap.find(key);
			if (pair != MaterialRootSignatureMap.end())
			{
				assert(pair->first == key);
				return pair->second;
			}

			ID3D12RootSignature* rootSignature = nullptr;

			switch (type)
			{
			case MaterialType::Opaque:
			{
				using params = GPass::OpaqueRootParameter;
				Helper::D3D12RootParameter parameters[params::Count]{};
				parameters[params::GlobalShaderData].AsCBV(D3D12_SHADER_VISIBILITY_ALL, 0);

				D3D12_SHADER_VISIBILITY bufferVisibility{};
				D3D12_SHADER_VISIBILITY dataVisibility{};

				if (flags & ShaderFlags::Vertex)
				{
					bufferVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
					dataVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
				}
				else if (flags & ShaderFlags::Mesh)
				{
					bufferVisibility = D3D12_SHADER_VISIBILITY_MESH;
					dataVisibility = D3D12_SHADER_VISIBILITY_MESH;
				}

				if ((flags & ShaderFlags::Hull) || (flags & ShaderFlags::Geometry) ||
					(flags & ShaderFlags::Amplification))
				{
					bufferVisibility = D3D12_SHADER_VISIBILITY_ALL;
					dataVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}

				if ((flags & ShaderFlags::Pixel) || (flags & ShaderFlags::Compute))
				{
					dataVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}

				parameters[params::PositionBuffer].AsSRV(bufferVisibility, 0);
				parameters[params::ElementBuffer].AsSRV(bufferVisibility, 1);
				parameters[params::SrvIndices].AsSRV(D3D12_SHADER_VISIBILITY_PIXEL, 2); // TODO: needs to be visible to any stages that need to sample textures.
				parameters[params::PerObjectData].AsCBV(dataVisibility, 1);

				rootSignature = Helper::D3D12RootSignatureDesc{ &parameters[0], _countof(parameters), GetRootSignatureFlags(flags) }.Create();
			}
			break;
			}

			assert(rootSignature);
			const ID::IDType id{ (ID::IDType)RootSignatures.size() };
			RootSignatures.emplace_back(rootSignature);
			MaterialRootSignatureMap[key] = id;
			SET_NAME_D3D12_OBJECT_INDEXED(rootSignature, key, L"GPass Root Signature - key");

			return id;
		}

		ID::IDType CreatePSO_IfNeeded(const uint8* const streamPtr, uint64 alignedStreamSize, [[maybe_unused]] bool isDepth)
		{
			const uint64 key = Math::CalcCRC32U64(streamPtr, alignedStreamSize);

			{
				std::lock_guard lock{ PSO_Mutex };
				auto pair = PSO_Map.find(key);

				if (pair != PSO_Map.end())
				{
					assert(pair->first == key);
					return pair->second;
				}
			}

			// 新しいPSOを作る処理はロックフリー
			Helper::D3D12PipelineStateSubobjectStream* const stream{ (Helper::D3D12PipelineStateSubobjectStream* const)streamPtr };
			ID3D12PipelineState* pso{ Helper::CreatePipelineState(stream, sizeof(Helper::D3D12PipelineStateSubobjectStream)) };

			{
				std::lock_guard lock{ PSO_Mutex };
				const ID::IDType id{ (uint32)PipelineStates.size() };
				PipelineStates.emplace_back(pso);
				SET_NAME_D3D12_OBJECT_INDEXED(PipelineStates.back(), key,
					isDepth ? L"Depth-only Pipeline State Object - key" : L"GPass Pipeline State Object - key");

				PSO_Map[key] = id;
				return id;
			}
		}

#pragma intrinsic(_BitScanForward)
		ShaderType::Type GetShaderType(uint32 flag)
		{
			assert(flag);
			unsigned long index;
			_BitScanForward(&index, flag);
			return (ShaderType::Type)index;
		}

		PSO_ID CreatePSO(ID::IDType materialID, D3D12_PRIMITIVE_TOPOLOGY primitiveTopology, uint32 elementsType)
		{

			constexpr uint64 alignedStreamSize{ Math::AlignSizeUp<sizeof(uint64)>(sizeof(Helper::D3D12PipelineStateSubobjectStream)) };
			uint8* const stream_ptr{ (uint8* const)alloca(alignedStreamSize) };
			ZeroMemory(stream_ptr, alignedStreamSize);
			new (stream_ptr) Helper::D3D12PipelineStateSubobjectStream{};

			Helper::D3D12PipelineStateSubobjectStream& stream{ *(Helper::D3D12PipelineStateSubobjectStream* const)stream_ptr };

			{
				std::lock_guard lock{ MaterialMutex };
				const D3D12MaterialStream material{ Materials[materialID].get() };

				D3D12_RT_FORMAT_ARRAY rtArray{};
				rtArray.NumRenderTargets = 1;
				rtArray.RTFormats[0] = GPass::MainBufferFormat;

				stream.RenderTargetFormats = rtArray;
				stream.rootSignature = RootSignatures[material.RootSignatureID()];
				stream.PrimitiveTopology = GetD3D_PrimitiveTopologyType(primitiveTopology);
				stream.DepthStencilFormat = GPass::DepthBufferFormat;
				stream.Rasterizer = Helper::RasterizerState.BackfaceCull;
				stream.DepthStencil1 = Helper::DepthState.EnabledReadonly;
				stream.Blend = Helper::BlendState.Disabled;

				const ShaderFlags::Flags flags{ material.ShaderFlags() };
				D3D12_SHADER_BYTECODE shaders[ShaderType::Count]{};
				uint32 shaderIndex = 0;
				for (uint32 i = 0; i < ShaderType::Count; i++)
				{
					if (flags & (1 << i))
					{
						const uint32 key{ GetShaderType(flags & (1 << i)) == ShaderType::Vertex ? elementsType : UINT32_INVALID_NUM };
						Rizityo::Content::CompiledShaderPtr shader{ Rizityo::Content::GetShader(material.ShaderIDs()[shaderIndex], key) };
						assert(shader);
						shaders[i].pShaderBytecode = shader->ByteCode();
						shaders[i].BytecodeLength = shader->ByteCodeSize();
						shaderIndex++;
					}
				}

				stream.VS = shaders[ShaderType::Vertex];
				stream.PS = shaders[ShaderType::Pixel];
				stream.DS = shaders[ShaderType::Domain];
				stream.HS = shaders[ShaderType::Hull];
				stream.GS = shaders[ShaderType::Geometry];
				stream.CS = shaders[ShaderType::Compute];
				stream.AS = shaders[ShaderType::Amplification];
				stream.MS = shaders[ShaderType::Mesh];
			}

			PSO_ID idPair{};
			idPair.GPassPSO_ID = CreatePSO_IfNeeded(stream_ptr, alignedStreamSize, false);

			stream.PS = D3D12_SHADER_BYTECODE{};
			stream.DepthStencil1 = Helper::DepthState.Reversed;
			idPair.DepthPSO_ID = CreatePSO_IfNeeded(stream_ptr, alignedStreamSize, true);

			return idPair;
		}

	} // 関数

	bool Initialize()
	{
		return true;
	}

	void Shutdown()
	{
		for (auto& item : RootSignatures)
		{
			Core::Release(item);
		}

		MaterialRootSignatureMap.clear();
		RootSignatures.clear();

		for (auto& item : PipelineStates)
		{
			Core::Release(item);
		}

		PSO_Map.clear();
		PipelineStates.clear();
	}

	namespace Submesh
	{

		// dataは以下の要素を含むことを仮定:
		//     uint32 ElementSize, uint32 VertexCount,
		//     uint32 IndexCount, uint32 elementsType, uint32 PrimitiveTopology
		//     uint8 Positions[sizeof(float32) * 3 * VertexCount], // sizeof(Positions)は4の倍数である必要
		//     uint8 Elements[sizeof(ElementSize) * VertexCount],  // sizeof(Elements)は4の倍数である必要
		//     uint8 Indices[IndexSize * IndexCount],
		//
		// Remark:
		// - 受け取ったdataポインタを進める
		// - 位置と頂点属性は4バイトの倍数である必要がある
		ID::IDType Add(const uint8*& data)
		{
			Utility::BinaryReader reader{ (const uint8*)data };

			const uint32 elementSize = reader.Read<uint32>();
			const uint32 vertexCount = reader.Read<uint32>();
			const uint32 indexCount = reader.Read<uint32>();
			const uint32 elementsType = reader.Read<uint32>();
			const uint32 primitiveTopology = reader.Read<uint32>();
			const uint32 indexSize = (vertexCount < (1 << 16)) ? sizeof(uint16) : sizeof(uint32);

			// 頂点位置だけの場合elementSizeは0になる
			const uint32 positionBufferSize = sizeof(Math::Vector3) * vertexCount;
			const uint32 elementBufferSize = elementSize * vertexCount;
			const uint32 indexBufferSize = indexSize * indexCount;

			constexpr uint32 alignment = D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE;
			const uint32 alignedPositionBufferSize = (uint32)Math::AlignSizeUp<alignment>(positionBufferSize);
			const uint32 alignedElementBufferSize = (uint32)Math::AlignSizeUp<alignment>(elementBufferSize);
			const uint32 totalBufferSize = alignedPositionBufferSize + alignedElementBufferSize + indexBufferSize;

			ID3D12Resource* resource = Helper::CreateBuffer(reader.Position(), totalBufferSize);

			reader.Skip(totalBufferSize);
			data = reader.Position();

			SubmeshView view{};
			view.PositionBufferView.BufferLocation = resource->GetGPUVirtualAddress();
			view.PositionBufferView.SizeInBytes = positionBufferSize;
			view.PositionBufferView.StrideInBytes = sizeof(Math::Vector3);

			if (elementSize)
			{
				view.ElementBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize;
				view.ElementBufferView.SizeInBytes = elementBufferSize;
				view.ElementBufferView.StrideInBytes = elementSize;
			}

			view.IndexBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize + alignedElementBufferSize;
			view.IndexBufferView.SizeInBytes = indexBufferSize;
			view.IndexBufferView.Format = (indexSize == sizeof(uint16)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;



			view.PrimitiveTopology = GetD3D_PrimitiveTopology((PrimitiveTopology::Type)primitiveTopology);
			view.ElementsType = elementsType;

			std::lock_guard lock{ SubmeshMutex };
			SubmeshBuffers.Add(resource);
			return SubmeshViews.Add(view);
		}

		void Remove(ID::IDType id)
		{
			std::lock_guard lock{ SubmeshMutex };
			SubmeshViews.Remove(id);

			Core::DeferredRelease(SubmeshBuffers[id]);
			SubmeshBuffers.Remove(id);
		}

		void GetViews(const ID::IDType* const gpuIDs, uint32 idCount, OUT const ViewsCache& cache)
		{
			assert(gpuIDs && idCount);
			assert(cache.PositionBuffers && cache.ElementBuffers && cache.IndexBufferViews &&
				cache.PrimitiveTopologies && cache.ElementsTypes);

			std::lock_guard lock{ SubmeshMutex };
			for (uint32 i = 0; i < idCount; i++)
			{
				const SubmeshView& view{ SubmeshViews[gpuIDs[i]] };
				cache.PositionBuffers[i] = view.PositionBufferView.BufferLocation;
				cache.ElementBuffers[i] = view.ElementBufferView.BufferLocation;
				cache.IndexBufferViews[i] = view.IndexBufferView;
				cache.PrimitiveTopologies[i] = view.PrimitiveTopology;
				cache.ElementsTypes[i] = view.ElementsType;
			}
		}

	} // Submesh

	namespace Texture {
		void GetDescriptorIndices(const ID::IDType* const textureIDs, uint32 idCount, OUT uint32* const indices)
		{
			assert(textureIDs && idCount && indices);
			std::lock_guard lock{ TextureMutex };
			for (uint32 i = 0; i < idCount; i++)
			{
				indices[i] = Textures[i].SRV().Index;
			}
		}
	}

	namespace Material
	{

		// 出力フォーマット:
		//
		// struct
		// {
		// MaterialType::Type Type,
		// ShaderFlags::Flags Flags,
		// ID::IDType RootSignatureID,
		// uint32 TextureCount,
		// ID::IDType ShaderIDs[ShaderCount],
		// ID::IDType TextureIDs[TextureCount],
		// uint32* DescriptorIndices[TextureCount]
		// } D3D12Material
		ID::IDType Add(MaterialInitInfo info)
		{
			std::unique_ptr<uint8[]> buffer;
			std::lock_guard lock{ MaterialMutex };
			D3D12MaterialStream stream{ buffer, info };
			assert(buffer);
			return Materials.Add(std::move(buffer));
		}

		void Remove(ID::IDType id)
		{
			std::lock_guard lock{ MaterialMutex };
			Materials.Remove(id);
		}

		void GetMaterials(const ID::IDType* const material_ids, uint32 material_count, OUT const MaterialsCache& cache)
		{
			assert(material_ids && material_count);
			assert(cache.RootSignatures && cache.MaterialTypes);
			std::lock_guard lock{ MaterialMutex };

			for (uint32 i = 0; i < material_count; i++)
			{
				const D3D12MaterialStream stream{ Materials[material_ids[i]].get() };
				cache.RootSignatures[i] = RootSignatures[stream.RootSignatureID()];
				cache.MaterialTypes[i] = stream.MaterialType();
			}
		}

	} // Material

	namespace RenderItem {

		// buffer[0] = geometryContentID
		// buffer[1 .. n] = D3D12RenderItemGPU_IDs (nはRenderItemの数を表す。マテリアル・サブメッシュIDの数と一致)
		// buffer[n + 1] = ID::INVALID_ID (サブメッシュのGPU ID列の終わりを表す)
		ID::IDType Add(ID::IDType entityID, ID::IDType geometryContentID,
			uint32 materialCount, const ID::IDType* const materialIDs)
		{
			assert(ID::IsValid(entityID) && ID::IsValid(geometryContentID));
			assert(materialCount && materialIDs);
			ID::IDType* const gpuIDs = (ID::IDType* const)alloca(materialCount * sizeof(ID::IDType));
			Rizityo::Content::GetSubmeshGPU_IDs(geometryContentID, materialCount, gpuIDs);

			Submesh::ViewsCache viewsCache
			{
				(D3D12_GPU_VIRTUAL_ADDRESS* const)alloca(materialCount * sizeof(D3D12_GPU_VIRTUAL_ADDRESS)),
				(D3D12_GPU_VIRTUAL_ADDRESS* const)alloca(materialCount * sizeof(D3D12_GPU_VIRTUAL_ADDRESS)),
				(D3D12_INDEX_BUFFER_VIEW* const)alloca(materialCount * sizeof(D3D12_INDEX_BUFFER_VIEW)),
				(D3D_PRIMITIVE_TOPOLOGY* const)alloca(materialCount * sizeof(D3D_PRIMITIVE_TOPOLOGY)),
				(uint32* const)alloca(materialCount * sizeof(uint32))
			};

			Submesh::GetViews(gpuIDs, materialCount, viewsCache);

			std::unique_ptr<ID::IDType[]> items{ std::make_unique<ID::IDType[]>(sizeof(ID::IDType) * (1 + (uint64)materialCount + 1)) };

			items[0] = geometryContentID;
			ID::IDType* const item_ids{ &items[1] };

			std::lock_guard lock{ RenderItemMutex };

			for (uint32 i = 0; i < materialCount; i++)
			{
				D3D12RenderItem item{};
				item.EntityID = entityID;
				item.SubmeshGPU_ID = gpuIDs[i];
				item.MaterialID = materialIDs[i];
				PSO_ID id_pair{ CreatePSO(item.MaterialID, viewsCache.PrimitiveTopologies[i], viewsCache.ElementsTypes[i]) };
				item.PSO_ID = id_pair.GPassPSO_ID;
				item.DepthPSO_ID = id_pair.DepthPSO_ID;

				assert(ID::IsValid(item.SubmeshGPU_ID) && ID::IsValid(item.MaterialID));
				item_ids[i] = RenderItems.Add(item);
			}

			// INVALID_IDで末尾を埋める
			item_ids[materialCount] = ID::INVALID_ID;

			return RenderItemIDs.Add(std::move(items));
		}

		void Remove(ID::IDType id)
		{
			std::lock_guard lock{ RenderItemMutex };
			const ID::IDType* const item_ids{ &RenderItemIDs[id][1] };

			for (uint32 i = 0; item_ids[i] != ID::INVALID_ID; i++)
			{
				RenderItems.Remove(item_ids[i]);
			}

			RenderItemIDs.Remove(id);
		}

		void GetD3D12RenderItemIDs(const FrameInfo& frameInfo, OUT Utility::Vector<ID::IDType>& d3d12RenderItemIDs)
		{
			assert(frameInfo.RenderItemIDs && frameInfo.Thresholds && frameInfo.RenderItemCount);
			assert(d3d12RenderItemIDs.empty());

			FrameCache.LOD_Offsets.clear();
			FrameCache.GeometryIDs.clear();
			const uint32 count = frameInfo.RenderItemCount;

			std::lock_guard lock{ RenderItemMutex };

			for (uint32 i = 0; i < count; i++)
			{
				const ID::IDType* const buffer = RenderItemIDs[frameInfo.RenderItemIDs[i]].get();
				FrameCache.GeometryIDs.emplace_back(buffer[0]);
			}

			Rizityo::Content::GetLOD_Offsets(FrameCache.GeometryIDs.data(), frameInfo.Thresholds, count, FrameCache.LOD_Offsets);
			assert(FrameCache.LOD_Offsets.size() == count);

			uint32 d3d12RenderItemCount = 0;
			for (uint32 i = 0; i < count; i++)
			{
				d3d12RenderItemCount += FrameCache.LOD_Offsets[i].Count;
			}

			assert(d3d12RenderItemCount);
			d3d12RenderItemIDs.resize(d3d12RenderItemCount);

			uint32 itemIndex = 0;
			for (uint32 i = 0; i < count; i++)
			{
				const ID::IDType* const itemIDs{ &RenderItemIDs[frameInfo.RenderItemIDs[i]][1] };
				const Rizityo::Content::LOD_Offset& lodOffset{ FrameCache.LOD_Offsets[i] };
				memcpy(&d3d12RenderItemIDs[itemIndex], &itemIDs[lodOffset.Offset], sizeof(ID::IDType) * lodOffset.Count);
				itemIndex += lodOffset.Count;
				assert(itemIndex <= d3d12RenderItemCount);
			}

			assert(itemIndex <= d3d12RenderItemCount);
		}

		void GetItems(const ID::IDType* const d3d12RenderItemIDs, uint32 idCount, OUT const ItemsCache& cache)
		{
			assert(d3d12RenderItemIDs && idCount);
			assert(cache.EntityIDs && cache.SubmeshGPU_IDs && cache.MaterialIDs &&
				cache.GPassPSOs && cache.DepthPSOs);

			std::lock_guard lock1{ RenderItemMutex };
			std::lock_guard lock2{ PSO_Mutex };

			for (uint32 i = 0; i < idCount; i++)
			{
				const D3D12RenderItem& item{ RenderItems[d3d12RenderItemIDs[i]] };
				cache.EntityIDs[i] = item.EntityID;
				cache.SubmeshGPU_IDs[i] = item.SubmeshGPU_ID;
				cache.MaterialIDs[i] = item.MaterialID;
				cache.GPassPSOs[i] = PipelineStates[item.PSO_ID];
				cache.DepthPSOs[i] = PipelineStates[item.DepthPSO_ID];
			}
		}

	} // RenderItem
}