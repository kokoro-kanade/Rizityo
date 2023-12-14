#include "Render.h"
#include "Content/AssetToEngine.h"
#include "Core/Utility/IO/FileIO.h"

namespace Rizityo::Render
{
	namespace
	{
		Vector<ID::IDType> RenderItemIDs;
		Vector<ID::IDType> RenderID_ItemIndex_Mapping; // RenderID��index -> RenderItemIDs��index
		std::unordered_map<ID::IDType, ID::IDType> ItemRenderID_Mapping; // RenderItemID -> RenderID

		Vector<float32> Thresholds;

		Vector<ID::GENERATION_TYPE> Generations;
		Deque<RenderID> FreeIds;

		std::unordered_map<size_t, ID::IDType> ModelID_Mapping;

		// �Q�[���N�����ɃR���p�C�����Ēǉ������
		// TODO? : ���炩���߃R���p�C�������V�F�[�_�[��ǂݍ���
		// hash(�t�@�C����/�֐���) -> �V�F�[�_�[ID
		std::unordered_map<size_t, ID::IDType> ShaderID_Mapping;

		std::unordered_map<size_t, ID::IDType> MaterialID_Mapping;

		using StringHash = std::hash<std::string>;
	}

	namespace
	{
		bool Exists(RenderID id)
		{
			assert(ID::IsValid(id));
			const ID::IDType index{ ID::GetIndex(id) };
			assert(index < Generations.size() && RenderID_ItemIndex_Mapping[index] < RenderItemIDs.size());
			return (Generations[index] == ID::GetGeneration(id) && ID::IsValid(RenderItemIDs[RenderID_ItemIndex_Mapping[index]]));
		}

		size_t HashShaderName(const char* fileName, const char* functionName)
		{
			std::string file{ fileName };
			file += '/';
			file += functionName;
			return StringHash()(file);
		}

		void LoadModel(const char* modelFilePath, size_t hash, OUT ID::IDType& modelID)
		{
			std::unique_ptr<uint8[]> model;
			uint64 size = 0;
			IO::ReadFile(modelFilePath, model, size);

			modelID = Content::CreateResource(model.get(), Content::AssetType::Mesh);
			assert(ID::IsValid(modelID));
			ModelID_Mapping[hash] = modelID;
		}
	}

	Render::Component CreateComponent(const InitInfo& info, GameEntity::Entity entity)
	{
		assert(entity.IsValid());

		RenderID id;
		if (FreeIds.size() > ID::MIN_DELETED_ELEMENTS) // FreeIds�����Ȃ���ԂŎg���܂킷�Ƃ�����generation��������Ă��܂��̂ł������l��݂���
		{
			id = FreeIds.front();
			assert(!Exists(id));
			FreeIds.pop_front();
			id = RenderID{ ID::IncrementGeneration(id) };
			Generations[ID::GetIndex(id)]++;
		}
		else
		{
			id = RenderID{ (ID::IDType)Generations.size() };
			Generations.push_back(0);
			RenderID_ItemIndex_Mapping.emplace_back();
		}

		assert(ID::IsValid(id));

		// modelID�쐬
		size_t modelFileHash = StringHash()(info.ModelName);
		ID::IDType modelID{};
		std::shared_ptr<std::thread> _model;
		if (ModelID_Mapping.find(modelFileHash) != ModelID_Mapping.end())
		{
			modelID = ModelID_Mapping[modelFileHash];
		}
		else
		{
			_model = std::make_shared<std::thread>([&] {LoadModel(info.ModelFilePath, modelFileHash, modelID); });
		}

		// materialID�쐬
		// TODO? : �}�e���A�����V�F�[�_�[�Ɠ��l�ɂ��炩���߃��[�h���Ă������ǂ���
		const uint32 materialCount = info.MaterialCount;
		Vector<ID::IDType> materials(materialCount);
		for (uint32 i = 0; i < materialCount; i++)
		{
			const MaterialInfo& mInfo{ info.MaterialsInfo[i] };
			size_t materialHash = StringHash()(mInfo.MaterialName);
			// ���łɃ}�e���A��������Ă���ꍇ�͂����p����
			if (MaterialID_Mapping.find(materialHash) != MaterialID_Mapping.end())
			{
				materials[i] = MaterialID_Mapping[materialHash];
				continue;
			}
			const ID::IDType vsID{ ShaderID_Mapping[HashShaderName(mInfo.ShadersInfo[ShaderType::Vertex].FileName, mInfo.ShadersInfo[ShaderType::Vertex].FunctionName)] };
			const ID::IDType psID{ ShaderID_Mapping[HashShaderName(mInfo.ShadersInfo[ShaderType::Pixel].FileName, mInfo.ShadersInfo[ShaderType::Pixel].FunctionName)] };
			Graphics::MaterialInitInfo materialInfo{};
			materialInfo.ShaderIDs[ShaderType::Vertex] = vsID;
			materialInfo.ShaderIDs[ShaderType::Pixel] = psID;
			materialInfo.Type = mInfo.Type;
			const ID::IDType materialID = Content::CreateResource(&materialInfo, Content::AssetType::Material);
			MaterialID_Mapping[materialHash] = materialID;
		}

		if (_model.get() != nullptr)
		{
			_model->join();
		}

		ID::IDType itemID{ Graphics::AddRenderItem(entity.ID(), modelID, materialCount, materials.data()) };
		RenderItemIDs.emplace_back(itemID);
		Thresholds.emplace_back(1.f); // TODO : thresholds�̕ύX

		const ID::IDType renderItemIndex{ (ID::IDType)RenderItemIDs.size() - 1 };
		RenderID_ItemIndex_Mapping[ID::GetIndex(id)] = renderItemIndex;
		ItemRenderID_Mapping[itemID] = id;
		return Render::Component{ id };
	}

	void RemoveComponent(Render::Component component)
	{
		assert(component.IsValid() && Exists(component.ID()));
		const RenderID id{ component.ID() };
		const ID::IDType itemIndex{ RenderID_ItemIndex_Mapping[ID::GetIndex(id)] };
		const ID::IDType itemID{ RenderItemIDs[itemIndex] };
		Graphics::RemoveRenderItem(itemID);
		const RenderID lastID{ (itemIndex != RenderItemIDs.size() - 1) ? ItemRenderID_Mapping[RenderItemIDs.back()] : id};
		EraseUnordered(RenderItemIDs, itemIndex);
		EraseUnordered(Thresholds, itemIndex);
		RenderID_ItemIndex_Mapping[ID::GetIndex(lastID)] = itemIndex;
		RenderID_ItemIndex_Mapping[ID::GetIndex(id)] = ID::INVALID_ID; // �v�f����̎���id == lastId�Ȃ̂�INVALID_ID�̑������
		FreeIds.push_back(id);
	}

	// TODO : Thresholds�̍X�V(�J�����̋����ɍ��킹��)
	void Update()
	{

	}

	void GetRenderFrameInfo(OUT Graphics::FrameInfo& info)
	{
		info.RenderItemIDs = RenderItemIDs.data();
		info.RenderItemCount = RenderItemIDs.size();
		info.Thresholds = Thresholds.data();
	}

	void AddShaderID(const char* fileName, const char* functionName, ID::IDType sID)
	{
		assert(ID::IsValid(sID));
		size_t hash = HashShaderName(fileName, functionName);
		assert(ShaderID_Mapping.find(hash) == ShaderID_Mapping.end());
		ShaderID_Mapping[hash] = sID;
	}

	void RemoveShader(const char* fileName, const char* functionName)
	{
		size_t hash = HashShaderName(fileName, functionName);
		assert(ShaderID_Mapping.find(hash) != ShaderID_Mapping.end());
		const ID::IDType sID{ ShaderID_Mapping[hash] };
		Content::RemoveShaderGroup(sID);
		ShaderID_Mapping.erase(hash);
	}

	// Mesh��Material�͒ǉ��̓G���e�B�e�B�̍쐬������
	// �폜�̓^�C�~���O�����R�Ɍ��߂���悤�ɂ���
	void RemoveModel(const char* modelName)
	{
		size_t hash = StringHash()(modelName);
		assert(ModelID_Mapping.find(hash) != ModelID_Mapping.end());
		const ID::IDType modelID{ ModelID_Mapping[hash] };
		assert(ID::IsValid(modelID));
		Content::DestroyResource(modelID, Content::AssetType::Mesh);
		ModelID_Mapping.erase(hash);
	}

	void RemoveMaterial(const char* materialName)
	{
		size_t hash = StringHash()(materialName);
		assert(MaterialID_Mapping.find(hash) != MaterialID_Mapping.end());
		const ID::IDType materialID{ MaterialID_Mapping[hash] };
		assert(ID::IsValid(materialID));
		Content::DestroyResource(materialID, Content::AssetType::Material);
		MaterialID_Mapping.erase(hash);
	}
}