#include "AssetToEngine.h"
#include "Graphics/Renderer.h"
#include "Core/Utility/IO/BinaryIO.h"

namespace Rizityo::Content
{
    namespace
    {
        class GeometryHierarchyStream
        {
        public:

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
                if (_LOD_Count == 1)
                    return 0;

                for (uint32 i = _LOD_Count - 1; i > 0; i--)
                {
                    if (_Thresholds[i] <= threshold)
                        return i;
                }

                //assert(false);
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
        FreeList<uint8*> GeometryHierarchies;
        std::mutex GeometryMutex;

        // std::vector用
        struct NoexceptMap {
            std::unordered_map<uint32, std::unique_ptr<uint8[]>> map;
            NoexceptMap() = default;
            NoexceptMap(const NoexceptMap&) = default;
            NoexceptMap(NoexceptMap&&) noexcept = default;
            NoexceptMap& operator=(const NoexceptMap&) = default;
            NoexceptMap& operator=(NoexceptMap&&) noexcept = default;
        };

        FreeList<NoexceptMap> ShaderGroups;
        std::mutex ShaderMutex;

    } // 変数

    namespace
    {
        // CreateGeometryResource()と同じデータフォーマットを入力に受け取る
        uint32 GetGeometryHierarchyBufferSize(const void* const data)
        {
            assert(data);
            IO::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);
            // LOD_Count, Thresholds, LOD Offsetsのサイズ
            uint32 size = sizeof(uint32) + (sizeof(float32) + sizeof(LOD_Offset)) * lodCount;

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

            IO::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);

            GeometryHierarchyStream stream{ hierarchyBuffer, lodCount };
            uint32 submeshIndex = 0;
            ID::IDType* const gpuIDs = stream.GPU_IDs();


            for (uint32 lodIndex = 0; lodIndex < lodCount; lodIndex++)
            {
                stream.Thresholds()[lodIndex] = reader.Read<float32>();
                const uint32 idCount = reader.Read<uint32>(); // SubmeshCount
                assert(idCount < (1 << 16));

                stream.LOD_Offsets()[lodIndex] = { (uint16)submeshIndex, (uint16)idCount };
                reader.Skip(sizeof(uint32)); // SizeOfSubmeshesは無視
                for (uint32 idIndex = 0; idIndex < idCount; idIndex++)
                {
                    const uint8* at{ reader.Position() };
                    gpuIDs[submeshIndex++] = Graphics::AddSubmesh(at);
                    reader.Skip((uint32)(at - reader.Position()));
                    assert(submeshIndex < (1 << 16));
                }
            }

            assert([&]() {
                float32 previous_threshold{ stream.Thresholds()[0] };
                for (uint32 i{ 1 }; i < lodCount; i++)
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
            IO::BinaryReader reader{ (const uint8*)data };
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
            IO::BinaryReader reader{ (const uint8*)data };
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

        constexpr ID::IDType GPU_ID_FromFakePointer(uint8* const pointer)
        {
            assert((uintptr_t)pointer & SingleMeshFlag);
            static_assert(sizeof(uintptr_t) > sizeof(ID::IDType));
            constexpr uint8 shiftBits{ (sizeof(uintptr_t) - sizeof(ID::IDType)) << 3 };
            return (((uintptr_t)pointer) >> shiftBits) & (uintptr_t)ID::INVALID_ID;
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
        //             uint8 Positions[sizeof(float32) * 3 * VertexCount], // sizeof(positions)は4の倍数である必要(必要ならパディング)
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
                const uint32 lodCount{ stream.LOD_Count() };
                uint32 idIndex = 0;
                // 全サブメッシュを削除
                for (uint32 lodIndex = 0; lodIndex < lodCount; lodIndex++)
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

        // dataは以下のフォーマットを仮定
        // struct {
        //  MaterialType::Type Type,
        //  uint32 TextureCount,
        //  ID::IDType ShaderIDs[ShaderType::Count],
        //  ID::IDType* TextureIDs;
        // } MaterialInitInfo
        ID::IDType CreateMaterialResource(const void* const data)
        {
            assert(data);
            return Graphics::AddMaterial(*(const Graphics::MaterialInitInfo* const)data);
        }

        void DestroyMaterialResource(ID::IDType id)
        {
            Graphics::RemoveMaterial(id);
        }

    } // 関数

    ID::IDType CreateResource(const void* const data, AssetType::Type type)
    {
        assert(data);

        ID::IDType id{ ID::INVALID_ID};

        switch (type)
        {
        case AssetType::Material:
            id = CreateMaterialResource(data);
            break;
        case AssetType::Mesh:
            id = CreateGeometryResource(data); 
            break;
        }

        assert(ID::IsValid(id));

        return id;
    }

    void DestroyResource(ID::IDType id, AssetType::Type type)
    {
        assert(ID::IsValid(id));
        switch (type)
        {
        case AssetType::Material:
            DestroyMaterialResource(id);
            break;
        case AssetType::Mesh:
            DestroyGeometryResource(id); 
            break;
        default:
            assert(false);
            break;
        }
    }

    ID::IDType AddShaderGroup(const uint8* const* shaders, uint32 numShaders, const uint32* const keys)
    {
        assert(shaders && numShaders && keys);

        NoexceptMap group;
        for (uint32 i = 0; i < numShaders; i++)
        {
            assert(shaders[i]);

            const CompiledShaderPtr shaderPtr{ (const CompiledShaderPtr)shaders[i] };
            const uint64 size = shaderPtr->BufferSize();
            std::unique_ptr<uint8[]> shader{ std::make_unique<uint8[]>(size) };
            memcpy(shader.get(), shaders[i], size);
            group.map[keys[i]] = std::move(shader);
        }
        std::lock_guard lock{ ShaderMutex };
        return ShaderGroups.Add(std::move(group));
    }

    void RemoveShaderGroup(ID::IDType id)
    {
        std::lock_guard lock{ ShaderMutex };
        assert(ID::IsValid(id));

        ShaderGroups[id].map.clear();
        ShaderGroups.Remove(id);
    }

    CompiledShaderPtr GetShader(ID::IDType id, uint32 shaderKey)
    {
        std::lock_guard lock{ ShaderMutex };
        assert(ID::IsValid(id));

        for (const auto& [key, value] : ShaderGroups[id].map)
        {
            if (key == shaderKey)
            {
                return (const CompiledShaderPtr)value.get();
            }
        }

        assert(false);
        return nullptr;
    }
    
    void GetSubmeshGPU_IDs(ID::IDType geometryContentID, uint32 idCount, OUT ID::IDType* const gpuIDs)
    {
        std::lock_guard lock{ GeometryMutex };
        uint8* const pointer = GeometryHierarchies[geometryContentID];
        if ((uintptr_t)pointer & SingleMeshFlag)
        {
            assert(idCount == 1);
            *gpuIDs = GPU_ID_FromFakePointer(pointer);
        }
        else
        {
            GeometryHierarchyStream stream{ pointer };

            assert([&]() {
                const uint32 lodCount{ stream.LOD_Count() };
                const LOD_Offset lodOffset{ stream.LOD_Offsets()[lodCount - 1] };
                const uint32 gpuID_Count{ (uint32)lodOffset.Offset + (uint32)lodOffset.Count };
                return gpuID_Count == idCount;
                }());

            memcpy(gpuIDs, stream.GPU_IDs(), sizeof(ID::IDType) * idCount);
        }
    }

    void GetLOD_Offsets(const ID::IDType* const geometryIDs, const float32* const thresholds, uint32 idCount, OUT Vector<LOD_Offset>& offsets)
    {
        assert(geometryIDs && thresholds && idCount);
        assert(offsets.empty());

        std::lock_guard lock{ GeometryMutex };

        for (uint32 i = 0; i < idCount; i++)
        {
            uint8* const pointer{ GeometryHierarchies[geometryIDs[i]] };
            if ((uintptr_t)pointer & SingleMeshFlag)
            {
                offsets.emplace_back(LOD_Offset{ 0, 1 });
            }
            else
            {
                GeometryHierarchyStream stream{ pointer };
                const uint32 lod{ stream.LOD_FromThreshold(thresholds[i]) };
                offsets.emplace_back(stream.LOD_Offsets()[lod]);
            }
        }
    }
}