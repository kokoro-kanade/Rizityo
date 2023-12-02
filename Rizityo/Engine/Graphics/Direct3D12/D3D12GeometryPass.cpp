#include "D3D12GeometryPass.h"
#include "D3D12Core.h"
#include "D3D12Shader.h"
#include "D3D12Content.h"
#include "D3D12Camera.h"
#include "D3D12Light.h"
#include "Shaders/SharedTypes.h"
#include "Components/Entity.h"
#include "Components/Transform.h"

namespace Rizityo::Graphics::D3D12::GPass
{
	namespace
	{
		constexpr Math::DX_U32Vector2 InitialDimensions{ 100,100 };

		D3D12RenderTexture GPassMainBuffer{};
		D3D12DepthBuffer GPassDepthBuffer{};
		Math::DX_U32Vector2 Dimensions{ InitialDimensions };

#if _DEBUG
		constexpr float32 ClearValue[4]{ 0.5f, 0.5f, 0.5f, 1.f };
#else
		constexpr float32 ClearValue[4]{};
#endif // _DEBUG

#if USE_STL_VECTOR
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

		struct GPassCache
		{
			Utility::Vector<ID::IDType> D3D12RenderItemIDs;

			// 配列で管理
			// 配列のサイズが変わるときはResizeする必要
			ID::IDType* EntityIDs = nullptr;
			ID::IDType* SubmeshGPU_IDs = nullptr;
			ID::IDType* MaterialIDs = nullptr;
			ID3D12PipelineState** GPassPipelineStates = nullptr;
			ID3D12PipelineState** DepthPipelineStates = nullptr;
			ID3D12RootSignature** RootSignatures = nullptr;
			MaterialType::Type* MaterialTypes = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS* PositionBuffers = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS* ElementBuffers = nullptr;
			D3D12_INDEX_BUFFER_VIEW* IndexBufferViews = nullptr;
			D3D_PRIMITIVE_TOPOLOGY* PrimitiveTopologies = nullptr;
			uint32* ElementsTypes = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS* PerObjectData = nullptr;

			constexpr Content::RenderItem::ItemsCache ItemsCache() const
			{
				return{
					EntityIDs,
					SubmeshGPU_IDs,
					MaterialIDs,
					GPassPipelineStates,
					DepthPipelineStates
				};
			}

			constexpr Content::Submesh::ViewsCache ViewsCache() const
			{
				return{
					PositionBuffers,
					ElementBuffers,
					IndexBufferViews,
					PrimitiveTopologies,
					ElementsTypes
				};
			}

			constexpr Content::Material::MaterialsCache MaterialsCache() const
			{
				return{
					RootSignatures,
					MaterialTypes
				};
			}

			CONSTEXPR uint32 Size() const
			{
				return (uint32)D3D12RenderItemIDs.size();
			}

			CONSTEXPR void Clear()
			{
				D3D12RenderItemIDs.clear();
			}

			CONSTEXPR void Resize()
			{
				const uint64 itemsCount = D3D12RenderItemIDs.size();
				const uint64 newBufferSize = itemsCount * StructSize;
				const uint64 oldBufferSize = _Buffer.size();
				if (newBufferSize > oldBufferSize)
				{
					_Buffer.resize(newBufferSize);
				}

				if (newBufferSize != oldBufferSize)
				{
					EntityIDs = (ID::IDType*)_Buffer.data();
					SubmeshGPU_IDs = (ID::IDType*)(&EntityIDs[itemsCount]);
					MaterialIDs = (ID::IDType*)(&SubmeshGPU_IDs[itemsCount]);
					GPassPipelineStates = (ID3D12PipelineState**)(&MaterialIDs[itemsCount]);
					DepthPipelineStates = (ID3D12PipelineState**)(&GPassPipelineStates[itemsCount]);
					RootSignatures = (ID3D12RootSignature**)(&DepthPipelineStates[itemsCount]);
					MaterialTypes = (MaterialType::Type*)(&RootSignatures[itemsCount]);
					PositionBuffers = (D3D12_GPU_VIRTUAL_ADDRESS*)(&MaterialTypes[itemsCount]);
					ElementBuffers = (D3D12_GPU_VIRTUAL_ADDRESS*)(&PositionBuffers[itemsCount]);
					IndexBufferViews = (D3D12_INDEX_BUFFER_VIEW*)(&ElementBuffers[itemsCount]);
					PrimitiveTopologies = (D3D_PRIMITIVE_TOPOLOGY*)(&IndexBufferViews[itemsCount]);
					ElementsTypes = (uint32*)(&PrimitiveTopologies[itemsCount]);
					PerObjectData = (D3D12_GPU_VIRTUAL_ADDRESS*)(&ElementsTypes[itemsCount]);
				}
			}

		private:
			constexpr static uint32 StructSize{
				sizeof(ID::IDType) +                   // EntityIDs
				sizeof(ID::IDType) +                   // SubmeshGPU_IDs
				sizeof(ID::IDType) +                   // MaterialIDs
				sizeof(ID3D12PipelineState*) +         // GPassPipelineStates
				sizeof(ID3D12PipelineState*) +         // DepthPipelineStates
				sizeof(ID3D12RootSignature*) +         // RootSignatures
				sizeof(MaterialType::Type) +           // MaterialTypes
				sizeof(D3D12_GPU_VIRTUAL_ADDRESS) +    // PositionBuffers
				sizeof(D3D12_GPU_VIRTUAL_ADDRESS) +    // ElementBuffers
				sizeof(D3D12_INDEX_BUFFER_VIEW) +      // IndexBufferViews
				sizeof(D3D_PRIMITIVE_TOPOLOGY) +       // PrimitiveTopologies
				sizeof(uint32) +                       // ElementsTypes
				sizeof(D3D12_GPU_VIRTUAL_ADDRESS)      // PerObjectData
			};

			Utility::Vector<uint8> _Buffer;
		} FrameCache;

#undef CONSTEXPR

	} // 変数

	namespace
	{
		bool CreateBuffers(Math::DX_U32Vector2 size)
		{
			assert(size.x && size.y);
			GPassMainBuffer.Release();
			GPassDepthBuffer.Release();

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0; // 0は64KB(MSAAでは4MB)を表す
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			desc.Format = MainBufferFormat;
			desc.Height = size.y;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 0; // 全てのミップレベル用のスペースを作る
			desc.SampleDesc = { 1,0 };
			desc.Width = size.x;

			// メインバッファ作成
			{
				D3D12TextureInitInfo info{};
				info.Desc = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				info.ClearValue.Format = desc.Format;
				memcpy(&info.ClearValue.Color, &ClearValue[0], sizeof(ClearValue));
				GPassMainBuffer = D3D12RenderTexture{ info };
			}

			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			desc.Format = DepthBufferFormat;
			desc.MipLevels = 1;

			// デプスバッファ作成
			{
				D3D12TextureInitInfo info{};
				info.Desc = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; // ピクセルシェーダーとコンピュートシェーダーで読み込む
				info.ClearValue.Format = desc.Format;
				info.ClearValue.DepthStencil.Depth = 0.f;
				info.ClearValue.DepthStencil.Stencil = 0;

				GPassDepthBuffer = D3D12DepthBuffer{ info };
			}

			SET_NAME_D3D12_OBJECT(GPassMainBuffer.Resource(), L"GPass Main Buffer");
			SET_NAME_D3D12_OBJECT(GPassDepthBuffer.Resource(), L"GPass Depth Buffer");

			return GPassMainBuffer.Resource() && GPassDepthBuffer.Resource();
		}

		void FillPerObjectData(const D3D12FrameInfo& d3d12Info)
		{
			const GPassCache& cache{ FrameCache };
			const uint32 renderItemsCount = (uint32)cache.Size();
			ID::IDType currentEntityID{ ID::INVALID_ID };
			HLSL::PerObjectData* currentDataPointer = nullptr;

			ConstantBuffer& cbuffer{ Core::GetConstantBuffer() };

			using namespace DirectX;
			for (uint32 i = 0; i < renderItemsCount; i++)
			{
				if (currentEntityID != cache.EntityIDs[i])
				{
					currentEntityID = cache.EntityIDs[i];
					HLSL::PerObjectData data{};
					Transform::GetTransformMatrices(GameEntity::EntityID{ currentEntityID }, data.World, data.InvWorld);
					XMMATRIX world{ XMLoadFloat4x4(&data.World) };
					XMMATRIX wvp{ XMMatrixMultiply(world, d3d12Info.Camera->ViewProjection()) };
					XMStoreFloat4x4(&data.WorldViewProjection, wvp);

					currentDataPointer = cbuffer.Allocate<HLSL::PerObjectData>();
					memcpy(currentDataPointer, &data, sizeof(HLSL::PerObjectData));
				}

				assert(currentDataPointer);
				cache.PerObjectData[i] = cbuffer.ToGPU_Address(currentDataPointer);
			}
		}

		void SetRootParameters(ID3D12GraphicsCommandList* const cmdList, uint32 cacheIndex)
		{
			GPassCache& cache{ FrameCache };
			assert(cacheIndex < cache.Size());

			const MaterialType::Type materialType{ cache.MaterialTypes[cacheIndex] };
			switch (materialType)
			{
			case MaterialType::Opaque:
			{
				using params = OpaqueRootParameter;
				cmdList->SetGraphicsRootShaderResourceView(params::PositionBuffer, cache.PositionBuffers[cacheIndex]);
				cmdList->SetGraphicsRootShaderResourceView(params::ElementBuffer, cache.ElementBuffers[cacheIndex]);
				cmdList->SetGraphicsRootConstantBufferView(params::PerObjectData, cache.PerObjectData[cacheIndex]);
			}
			break;
			}
		}

		void PrepareRenderFrame(const D3D12FrameInfo& d3d12Info)
		{
			assert(d3d12Info.FrameInfo && d3d12Info.Camera);
			assert(d3d12Info.FrameInfo->RenderItemIDs && d3d12Info.FrameInfo->RenderItemCount);

			GPassCache& cache{ FrameCache };
			cache.Clear();

			using namespace Content;
			RenderItem::GetD3D12RenderItemIDs(*d3d12Info.FrameInfo, cache.D3D12RenderItemIDs);
			cache.Resize();
			const uint32 itemsCount = cache.Size();

			const RenderItem::ItemsCache itemsCache{ cache.ItemsCache() };
			RenderItem::GetItems(cache.D3D12RenderItemIDs.data(), itemsCount, itemsCache);

			const Submesh::ViewsCache viewsCache{ cache.ViewsCache() };
			Submesh::GetViews(itemsCache.SubmeshGPU_IDs, itemsCount, viewsCache);

			const Material::MaterialsCache materialsCache{ cache.MaterialsCache() };
			Material::GetMaterials(itemsCache.MaterialIDs, itemsCount, materialsCache);

			FillPerObjectData(d3d12Info);
		}

	} // 関数

	bool Initialize()
	{
		return CreateBuffers(InitialDimensions);
	}

	void Shutdown()
	{
		GPassMainBuffer.Release();
		GPassDepthBuffer.Release();
		Dimensions = InitialDimensions;
	}

	const D3D12RenderTexture& GetMainBuffer()
	{
		return GPassMainBuffer;
	}

	const D3D12DepthBuffer& GetDepthBuffer()
	{
		return GPassDepthBuffer;
	}

	void SetSize(Math::DX_U32Vector2 size)
	{
		Math::DX_U32Vector2& d{ Dimensions };
		if (size.x > d.x || size.y > d.y)
		{
			d = { std::max(size.x, d.x), std::max(size.y, d.y) };
			CreateBuffers(d);
		}
	}

	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info)
	{
		PrepareRenderFrame(d3d12Info);

		const GPassCache& cache{ FrameCache };
		const uint32 itemsCount = cache.Size();

		ID3D12RootSignature* currentRootSignature = nullptr;
		ID3D12PipelineState* currentPipelineState = nullptr;

		for (uint32 i = 0; i < itemsCount; i++)
		{
			if (currentRootSignature != cache.RootSignatures[i])
			{
				currentRootSignature = cache.RootSignatures[i];
				cmdList->SetGraphicsRootSignature(currentRootSignature);
				cmdList->SetGraphicsRootConstantBufferView(OpaqueRootParameter::GlobalShaderData, d3d12Info.GlobalShaderData);
			}

			if (currentPipelineState != cache.DepthPipelineStates[i])
			{
				currentPipelineState = cache.DepthPipelineStates[i];
				cmdList->SetPipelineState(currentPipelineState);
			}

			SetRootParameters(cmdList, i);

			const D3D12_INDEX_BUFFER_VIEW& ibv{ cache.IndexBufferViews[i] };
			const uint32 indexCount = ibv.SizeInBytes >> (ibv.Format == DXGI_FORMAT_R16_UINT ? 1 : 2);

			cmdList->IASetIndexBuffer(&ibv);
			cmdList->IASetPrimitiveTopology(cache.PrimitiveTopologies[i]);
			cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
		}
	}

	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& d3d12Info)
	{
		const GPassCache& cache{ FrameCache };
		const uint32 itemsCount = cache.Size();

		ID3D12RootSignature* currentRootSignature = nullptr;
		ID3D12PipelineState* currentPipelineState = nullptr;

		for (uint32 i = 0; i < itemsCount; i++)
		{
			if (currentRootSignature != cache.RootSignatures[i])
			{
				currentRootSignature = cache.RootSignatures[i];
				cmdList->SetGraphicsRootSignature(currentRootSignature);
				cmdList->SetGraphicsRootConstantBufferView(OpaqueRootParameter::GlobalShaderData, d3d12Info.GlobalShaderData);
				cmdList->SetGraphicsRootShaderResourceView(OpaqueRootParameter::directional_lights,
														   Light::GetNonCullableLightBuffer(d3d12Info.FrameIndex));
			}

			if (currentPipelineState != cache.GPassPipelineStates[i])
			{
				currentPipelineState = cache.GPassPipelineStates[i];
				cmdList->SetPipelineState(currentPipelineState);
			}

			SetRootParameters(cmdList, i);

			const D3D12_INDEX_BUFFER_VIEW& ibv{ cache.IndexBufferViews[i] };
			const uint32 indexCount = ibv.SizeInBytes >> (ibv.Format == DXGI_FORMAT_R16_UINT ? 1 : 2);

			cmdList->IASetIndexBuffer(&ibv);
			cmdList->IASetPrimitiveTopology(cache.PrimitiveTopologies[i]);
			cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
		}
	}

	void AddTransitionsForDepthPrepass(Helper::D3D12ResourceBarrier& barriers)
	{
		barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
		barriers.Add(GPassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE);

	}

	void AddTransitionsForGPass(Helper::D3D12ResourceBarrier& barriers)
	{
		barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);
		barriers.Add(GPassDepthBuffer.Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void AddTransitionsForPostProcess(Helper::D3D12ResourceBarrier& barriers)
	{
		barriers.Add(GPassMainBuffer.Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV() };

		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(0, nullptr, 0, &dsv);
	}

	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ GPassMainBuffer.RTV(0) };
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV() };

		cmdList->ClearRenderTargetView(rtv, ClearValue, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, 0, &dsv);
	}
}