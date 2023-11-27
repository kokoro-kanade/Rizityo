#include <filesystem>
#include "CommonHeaders.h"
#include "Content/ContentToEngine.h"
#include "ShaderCompile.h"
#include "Components/Entity.h"
#include "Graphics/Renderer.h"

using namespace Rizityo;

bool ReadFile(std::filesystem::path, OUT std::unique_ptr<uint8[]>&, OUT uint64&);

namespace
{

    ID::IDType ModelID{ ID::INVALID_ID};
    ID::IDType VS_ID{ ID::INVALID_ID };
    ID::IDType PS_ID{ ID::INVALID_ID };
    ID::IDType MaterialID{ ID::INVALID_ID};

    std::unordered_map<ID::IDType, ID::IDType> RenderItemEntityMap;

    void LoadModel()
    {
        std::unique_ptr<uint8[]> model;
        uint64 size = 0;
        ReadFile("..\\..\\Test\\test.model", model, size);

        ModelID = Content::CreateResource(model.get(), Content::AssetType::Mesh);
        assert(ID::IsValid(ModelID));
    }

    void LoadShaders()
    {
        ShaderFileInfo info{};
        info.FileName = "TestShader.hlsl";
        info.FunctionName = "TestShaderVS";
        info.Type = ShaderType::Vertex;

        const char* shaderPath{ "..\\..\\Test\\" };

        auto vertexShader = CompileShader(info, shaderPath);
        assert(vertexShader.get());

        info.FunctionName = "TestShaderPS";
        info.Type = ShaderType::Pixel;

        auto pixelShader = CompileShader(info, shaderPath);
        assert(pixelShader.get());

        VS_ID = Content::AddShader(vertexShader.get());
        PS_ID = Content::AddShader(pixelShader.get());
    }

    void CreateMaterial()
    {
        Graphics::MaterialInitInfo info{};
        info.ShaderIDs[Graphics::ShaderType::Vertex] = VS_ID;
        info.ShaderIDs[Graphics::ShaderType::Pixel] = PS_ID;
        info.Type = Graphics::MaterialType::Opaque;
        MaterialID = Content::CreateResource(&info, Content::AssetType::Material);
    }

} // 無名空間

ID::IDType CreateRenderItem(ID::IDType entityID)
{
    // モデルのロード
    auto _1 = std::thread{ [] { LoadModel(); } };

    // マテリアルのロード
    // 1) テクスチャ
    // 2) シェーダー
    auto _2 = std::thread{ [] { LoadShaders(); } };

    _1.join();
    _2.join();

    // エンティティとマテリアルとモデルをひとまとめにしてRenderItem作成
    CreateMaterial();
    ID::IDType materials[]{ MaterialID, MaterialID, MaterialID, MaterialID, MaterialID };
    ID::IDType itemID{ Graphics::AddRenderItem(entityID, ModelID, _countof(materials), &materials[0]) };

    RenderItemEntityMap[itemID] = entityID;
    return itemID;
}

void DestroyRenderItem(ID::IDType itemID)
{
    // RenderItemとゲームエンティティの削除
    if (ID::IsValid(itemID))
    {
        Graphics::RemoveRenderItem(itemID);
        auto pair = RenderItemEntityMap.find(itemID);
        if (pair != RenderItemEntityMap.end())
        {
            GameEntity::RemoveGameEnity(GameEntity::EntityID{ pair->second });
        }
    }

    // マテリアルの削除
    if (ID::IsValid(MaterialID))
    {
        Content::DestroyResource(MaterialID, Content::AssetType::Material);
    }

    // シェーダーとテクスチャの削除
    if (ID::IsValid(VS_ID))
    {
        Content::RemoveShader(VS_ID);
    }

    if (ID::IsValid(PS_ID))
    {
        Content::RemoveShader(PS_ID);
    }

    // モデルの削除
    if (ID::IsValid(ModelID))
    {
        Content::DestroyResource(ModelID, Content::AssetType::Mesh);
    }
}