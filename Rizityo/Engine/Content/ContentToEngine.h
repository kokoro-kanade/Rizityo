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

    struct PrimitiveTopology {
        enum Type : uint32 {
            PointList = 1,
            LineList,
            LineStrip,
            TriangleList,
            TriangleStrip,
            Count
        };
    };

    ID::IDType CreateResource(const void* const data, AssetType::Type type);
    void DestroyResource(ID::IDType id, AssetType::Type type);
}