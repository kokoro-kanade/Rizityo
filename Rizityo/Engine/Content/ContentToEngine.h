#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Content {

    struct AssetType {
        enum Type : uint32 {
            Unknown = 0,
            Animation,
            Audio,
            Material,
            Mesh,
            Skeleton,
            Texture,
            Count
        };
    };

    ID::IDType CreateResource(const void* const data, AssetType::Type type);
    void DestroyResource(ID::IDType id, AssetType::Type type);


    typedef struct CompiledShader
    {
        static constexpr uint32 HashLength = 16;
        constexpr uint64 ByteCodeSize() const { return _ByteCodeSize; }
        constexpr const uint8* const Hash() const { return &_Hash[0]; }
        constexpr const uint8* const ByteCode() const { return &_ByteCode; }
    private:
        uint64         _ByteCodeSize;
        uint8          _Hash[HashLength];
        uint8          _ByteCode;
    } const* CompiledShaderPtr;

    ID::IDType AddShader(const uint8* data);
    void RemoveShader(ID::IDType id);
    CompiledShaderPtr GetShader(ID::IDType id);

    struct LOD_Offset
    {
        uint16 Offset;
        uint16 Count;
    };

    void GetSubmeshGPU_IDs(ID::IDType geometryContentID, uint32 idCount, OUT ID::IDType* const gpuIDs);
    void GetLOD_Offsets(const ID::IDType* const geometryIds, const float32* const thresholds, uint32 idCount, OUT Utility::Vector<LOD_Offset>& offsets);
}