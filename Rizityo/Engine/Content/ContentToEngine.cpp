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

        constexpr uintptr_t SingleMeshFlag{ (uintptr_t)0x01 }; // GeometryHierarchies�̗v�f�ɂ��̃t���O�������Ă���ꍇ��GPU ID�ł��邱�Ƃ�\��
        Utility::FreeList<uint8*> GeometryHierarchies;
        std::mutex GeometryMutex;

    } // �ϐ�

    namespace
    {
        // CreateGeometryResource()�Ɠ����f�[�^�t�H�[�}�b�g����͂Ɏ󂯎��
        uint32 GetGeometryHierarchyBufferSize(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);
            // LOD_Count, Thresholds, LOD Offsets�̃T�C�Y
            uint32 size = sizeof(uint32) + (sizeof(float32) + sizeof(GeometryHierarchyStream::LOD_Offset)) * lodCount;

            for (uint32 lodIndex = 0; lodIndex < lodCount; lodIndex++)
            {
                // threshold�͖���
                reader.Skip(sizeof(float32));
                // GPU_IDs�̃T�C�Y (sizeof(ID::IDType) * SubmeshCount)
                size += sizeof(ID::IDType) * reader.Read<uint32>();
                // �T�u���b�V���͖���
                reader.Skip(reader.Read<uint32>());
            }

            return size;
        }

        // ������LOD��T�u���b�V�������W�I���g���p��HierarchyStream�쐬
        // CreateGeometryResource()�Ɠ����f�[�^�t�H�[�}�b�g����͂Ɏ󂯎��
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
                reader.Skip(sizeof(uint32)); // SizeOfSubmeshes�͖���
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

            static_assert(alignof(void*) > 2, "�ŉ��ʃr�b�g��SingleMeshFlag���K�v�ł�");
            std::lock_guard lock{ GeometryMutex };
            return GeometryHierarchies.Add(hierarchyBuffer);
        }

        // ��̃T�u���b�V����GPU ID���쐬
        // CreateGeometryResource()�Ɠ����f�[�^�t�H�[�}�b�g����͂Ɏ󂯎��
        ID::IDType CreateSingleSubmesh(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            // LOD_Count, LOD_Threshold, SubmeshCount, SizeOfSubmeshes�𖳎�
            reader.Skip(sizeof(uint32) + sizeof(float32) + sizeof(uint32) + sizeof(uint32));
            const uint8* at = reader.Position();
            const ID::IDType gpuID{ Graphics::AddSubmesh(at) };

            // �t�F�C�N�|�C���^���Ƀf�[�^���i�[
            static_assert(sizeof(uintptr_t) > sizeof(ID::IDType));
            constexpr uint8 shiftBits{ (sizeof(uintptr_t) - sizeof(ID::IDType)) << 3 };
            uint8* const fakePointer{ (uint8* const)((((uintptr_t)gpuID) << shiftBits) | SingleMeshFlag) };
            std::lock_guard lock{ GeometryMutex };
            return GeometryHierarchies.Add(fakePointer);
        }

        // �W�I���g�������LOD�ň�̃T�u���b�V�����������Ȃ����ǂ���
        // CreateGeometryResource()�Ɠ����f�[�^�t�H�[�}�b�g����͂Ɏ󂯎��
        bool IsSingleMesh(const void* const data)
        {
            assert(data);
            Utility::BinaryReader reader{ (const uint8*)data };
            const uint32 lodCount = reader.Read<uint32>();
            assert(lodCount);
            if (lodCount > 1)
                return false;

            // threshold�͖���
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

        // data�͈ȉ��̃t�H�[�}�b�g������
        // struct{
        //     uint32 LOD_Count,
        //     struct {
        //         float32 LOD_Threshold,
        //         uint32 SubmeshCount,
        //         uint32 SizeOfSubmeshes,
        //         struct {
        //             uint32 ElementSize, uint32 VertexCount,
        //             uint32 IndexCount, uint32 ElementsType, uint32 PrimitiveTopology
        //             uint8 Positions[sizeof(float32) * 3 * VertexCount],     // sizeof(positions)��4�̔{���ł���K�v(�K�v�Ȃ�p�f�B���O)
        //             uint8 Elements[sizeof(ElementSize) * VertexCount], // sizeof(elements)��4�̔{���ł���K�v(�K�v�Ȃ�p�f�B���O)
        //             uint8 Indices[IndexSize * IndexCount]
        //         } Submeshes[SubmeshCount]
        //     } MeshLODs[LOD_Count]
        // } Geometry;
        //
        // �o�̓t�H�[�}�b�g
        //
        // LOD���T�u���b�V������ȏ゠��ꍇ
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
        // LOD����ŃT�u���b�V������̏ꍇ
        //
        // (GPU_ID << 32) | 0x01(�����Ƀt���O)
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
                // �S�T�u���b�V�����폜
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

    } // �֐�

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