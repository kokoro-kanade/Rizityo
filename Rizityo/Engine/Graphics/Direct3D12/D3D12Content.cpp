#include "D3D12Content.h"
#include "D3D12Core.h"
#include "Utility/IOStream.h"
#include "Content/ContentToEngine.h"

namespace Rizityo::Graphics::D3D12::Content
{
    namespace
    {

        struct SubmeshView
        {
            D3D12_VERTEX_BUFFER_VIEW        PositionBufferView{};
            D3D12_VERTEX_BUFFER_VIEW        ElementBufferView{};
            D3D12_INDEX_BUFFER_VIEW         IndexBufferView{};
            D3D_PRIMITIVE_TOPOLOGY          PrimitiveTopology;
            uint32                             ElementsType{};
        };

        Utility::FreeList<ID3D12Resource*>     SubmeshBuffers{};
        Utility::FreeList<SubmeshView>        SubmeshViews{};
        std::mutex                          SubmeshMutex{};

        D3D_PRIMITIVE_TOPOLOGY GetD3D_PrimitiveTopology(Rizityo::Content::PrimitiveTopology::Type type)
        {
            using namespace Rizityo::Content;
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

    } // 無名空間

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



            view.PrimitiveTopology = GetD3D_PrimitiveTopology((Rizityo::Content::PrimitiveTopology::Type)primitiveTopology);
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

    } // namespace Submesh
}