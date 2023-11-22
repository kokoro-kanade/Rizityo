#include "ContentToEngine.h"
#include "Graphics/Renderer.h"
#include "Utility/IOStream.h"

namespace Rizityo::Content
{
    namespace
    {
        class GeometryHierarchyStream
        {
        public:
            struct LOD_Offset
            {
                uint16 Offset;
                uint16 Count;
            };

            DISABLE_COPY_AND_MOVE(GeometryHierarchyStream);
            GeometryHierarchyStream(uint8* const buffer, uint32 lods = UINT32_INVALID_NUM)
                : _Buffer{ buffer }
            {
                assert(buffer && lods);
                if (lods != UINT32_INVALID_NUM)
                {
                    *((uint32*)buffer) = lods;
                }

                _LOD_Count = *((uint32*)buffer);
                _Thresholds = (float32*)(&buffer[sizeof(uint32)]);
                _LOD_Offsets = (LOD_Offset*)(&_Thresholds[_LOD_Count]);
                _GPU_IDs = (ID::IDType*)(&_LOD_Offsets[_LOD_Count]);
            }

            void GPU_IDs(uint32 lod, OUT ID::IDType*& ids, OUT uint32& idCount)
            {
                assert(lod < _LOD_Count);
                ids = &_GPU_IDs[_LOD_Offsets[lod].Offset];
                idCount = _LOD_Offsets[lod].Count;
            }

            uint32 LOD_FromThreshold(float32 threshold)
            {
                assert(threshold > 0);

                for (uint32 i{ _LOD_Count - 1 }; i > 0; --i)
                {
                    if (_Thresholds[i] <= threshold) return i;
                }

                assert(false); // shouldn't ever get here.
                return 0;
            }

            [[nodiscard]] constexpr uint32 LOD_Count() const { return _LOD_Count; }
            [[nodiscard]] constexpr float32* Thresholds() const { return _Thresholds; }
            [[nodiscard]] constexpr LOD_Offset* LOD_Offsets() const { return _LOD_Offsets; }
            [[nodiscard]] constexpr ID::IDType* GPU_IDs() const { return _GPU_IDs; }

        private:
            uint8* const _Buffer;
            float32* _Thresholds;
            LOD_Offset* _LOD_Offsets;
            ID::IDType* _GPU_IDs;
            uint32 _LOD_Count;
        };

        constexpr uintptr_t SingleMeshFlag{ (uintptr_t)0x01 }; // GeometryHierarchiesの要素にこのフラグが立っている場合はGPU IDであることを表す
        Utility::FreeList<uint8*> GeometryHierarchies;
        std::mutex GeometryMutex;

    } // 変数

    namespace
    {
        // CreateGeometryResource()と同じデータフォーマットを入力に受け取る
        uint32 GetGeometryHierarchyBufferSize(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);
            // LOD_Count, Thresholds, LOD Offsetsのサイズ
            uint32 size = sizeof(uint32) + (sizeof(float32) + sizeof(GeometryHierarchyStream::LOD_Offset)) * lodCount;

            for (uint32 lodIndex = 0; lodIndex < lodCount; lodIndex++)
            {
                // thresholdは無視
                reader.Skip(sizeof(float32));
                // GPU_IDsのサイズ (sizeof(ID::IDType) * SubmeshCount)
                size += sizeof(ID::IDType) * reader.Read<uint32>();
                // サブメッシュは無視
                reader.Skip(reader.Read<uint32>());
            }

            return size;
        }

        // 複数のLODやサブメッシュを持つジオメトリ用のHierarchyStream作成
        // CreateGeometryResource()と同じデータフォーマットを入力に受け取る
        ID::IDType CreateMeshHierarchy(const void* const data)
        {
            assert(data);

            const uint32 size = GetGeometryHierarchyBufferSize(data);
            uint8* const hierarchyBuffer = (uint8* const)malloc(size);

            Utility::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);

            GeometryHierarchyStream stream{ hierarchyBuffer, lodCount };
            uint32 submeshIndex = 0;
            ID::IDType* const gpuIDs = stream.GPU_IDs();

            for (uint32 lodIndex = 0; lodIndex < lodCount; lodIndex++)
            {
                stream.Thresholds()[lodIndex] = reader.Read<float32>();
                const uint32 idCount = reader.Read<uint32>();
                assert(idCount < (1 << 16));

                stream.LOD_Offsets()[lodIndex] = { (uint16)submeshIndex, (uint16)idCount };
                reader.Skip(sizeof(uint32)); // SizeOfSubmeshesは無視
                for (uint32 idIndex = 0; idIndex < idCount; ++idIndex)
                {
                    const uint8* at{ reader.Position() };
                    gpuIDs[submeshIndex++] = Graphics::AddSubmesh(at);
                    reader.Skip((uint32)(at - reader.Position()));
                    assert(submeshIndex < (1 << 16));
                }
            }

            assert([&]() {
                float32 previous_threshold{ stream.Thresholds()[0] };
                for (uint32 i{ 1 }; i < lodCount; ++i)
                {
                    if (stream.Thresholds()[i] <= previous_threshold) return false;
                    previous_threshold = stream.Thresholds()[i];
                }
                return true;
                }());

            static_assert(alignof(void*) > 2, "最下位ビットにSingleMeshFlagが必要です");
            std::lock_guard lock{ GeometryMutex };
            return GeometryHierarchies.Add(hierarchyBuffer);
        }

        // 一つのサブメッシュのGPU IDを作成
        // CreateGeometryResource()と同じデータフォーマットを入力に受け取る
        ID::IDType CreateSingleSubmesh(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            // LOD_Count, LOD_Threshold, SubmeshCount, SizeOfSubmeshesを無視
            reader.Skip(sizeof(uint32) + sizeof(float32) + sizeof(uint32) + sizeof(uint32));
            const uint8* at = reader.Position();
            const ID::IDType gpuID{ Graphics::AddSubmesh(at) };

            // フェイクポインタ内にデータを格納
            static_assert(sizeof(uintptr_t) > sizeof(ID::IDType));
            constexpr uint8 shiftBits{ (sizeof(uintptr_t) - sizeof(ID::IDType)) << 3 };
            uint8* const fakePointer{ (uint8* const)((((uintptr_t)gpuID) << shiftBits) | SingleMeshFlag) };
            std::lock_guard lock{ GeometryMutex };
            return GeometryHierarchies.Add(fakePointer);
        }

        // ジオメトリが一つのLODで一つのサブメッシュしか持たないかどうか
        // CreateGeometryResource()と同じデータフォーマットを入力に受け取る
        bool IsSingleMesh(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);
            if (lodCount > 1)
                return false;

            // thresholdは無視
            reader.Skip(sizeof(float32));
            const uint32 submeshCount = reader.Read<uint32>();
            assert(submeshCount);
            return submeshCount == 1;
        }

        ID::IDType GPU_ID_FromFakePointer(uint8* const pointer)
        {
            assert((uintptr_t)pointer & SingleMeshFlag);
            static_assert(sizeof(uintptr_t) > sizeof(ID::IDType));
            constexpr uint8 shift_bits{ (sizeof(uintptr_t) - sizeof(ID::IDType)) << 3 };
            return (((uintptr_t)pointer) >> shift_bits) & (uintptr_t)ID::INVALID_ID;
        }

        // dataは以下のフォーマットを仮定
        // struct{
        //     uint32 LOD_Count,
        //     struct {
        //         float32 LOD_Threshold,
        //         uint32 SubmeshCount,
        //         uint32 SizeOfSubmeshes,
        //         struct {
        //             uint32 ElementSize, uint32 VertexCount,
        //             uint32 IndexCount, uint32 ElementsType, uint32 PrimitiveTopology
        //             uint8 Positions[sizeof(float32) * 3 * VertexCount],     // sizeof(positions)は4の倍数である必要(必要ならパディング)
        //             uint8 Elements[sizeof(ElementSize) * VertexCount], // sizeof(elements)は4の倍数である必要(必要ならパディング)
        //             uint8 Indices[IndexSize * IndexCount]
        //         } Submeshes[SubmeshCount]
        //     } MeshLODs[LOD_Count]
        // } Geometry;
        //
        // 出力フォーマット
        //
        // LODかサブメッシュが二つ以上ある場合
        // struct {
        //     uint32 LOD_Count,
        //     float32 Thresholds[LOD_Count]
        //     struct {
        //         uint16 Offset,
        //         uint16 Count
        //     } LOD_Offsets[LOD_Count],
        //     ID::IDType GPU_IDs[TotalNumberOfSubmeshes]
        // } GeometryHierarchy
        // 
        // LODが一つでサブメッシュも一つの場合
        //
        // (GPU_ID << 32) | 0x01(末尾にフラグ)
        //
        ID::IDType CreateGeometryResource(const void* const data)
        {
            assert(data);
            return IsSingleMesh(data) ? CreateSingleSubmesh(data) : CreateMeshHierarchy(data);
        }

        void DestroyGeometryResource(ID::IDType id)
        {
            std::lock_guard lock{ GeometryMutex };
            uint8* const  pointer = GeometryHierarchies[id];
            if ((uintptr_t)pointer & SingleMeshFlag)
            {
                Graphics::RemoveSubmesh(GPU_ID_FromFakePointer(pointer));
            }
            else
            {
                GeometryHierarchyStream stream{ pointer };
                const uint32 lod_count{ stream.LOD_Count() };
                uint32 idIndex = 0;
                // 全サブメッシュを削除
                for (uint32 lodIndex = 0; lodIndex < lod_count; lodIndex++)
                {
                    for (uint32 i = 0; i < stream.LOD_Offsets()[lodIndex].Count; i++)
                    {
                        Graphics::RemoveSubmesh(stream.GPU_IDs()[idIndex++]);
                    }
                }

                free(pointer);
            }

            GeometryHierarchies.Remove(id);
        }

    } // 関数

    ID::IDType CreateResource(const void* const data, AssetType::Type type)
    {
        assert(data);

        ID::IDType id{ ID::INVALID_ID};

        switch (type)
        {
        case AssetType::Animation: break;
        case AssetType::Audio:	break;
        case AssetType::Material: break;
        case AssetType::Mesh:
            id = CreateGeometryResource(data); 
            break;
        case AssetType::Skeleton: break;
        case AssetType::Texture: break;
        }

        assert(ID::IsValid(id));

        return id;
    }

    void DestroyResource(ID::IDType id, AssetType::Type type)
    {
        assert(ID::IsValid(id));
        switch (type)
        {
        case AssetType::Animation: break;
        case AssetType::Audio:	break;
        case AssetType::Material: break;
        case AssetType::Mesh:
            DestroyGeometryResource(id); 
            break;
        case AssetType::Skeleton: break;
        case AssetType::Texture: break;
        default:
            assert(false);
            break;
        }
    }
}