#pragma once
#include "Test.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/Transform.h"

#include <iostream>
#include <ctime>

using namespace Rizityo;

class EngineTest : public Test
{
public:
	bool Initialize() override
	{
		srand((uint32)time(nullptr));
		return true;
	}

	void Run() override
	{
		do
		{
			for (uint32 i = 0; i < 10000; i++)
			{
				CreateRandomNumEntity();
				RemoveRandomNumEntity();
				EntitiesNum = (uint32)Entities.size();
			}
			PrintResults();
		} while (getchar() != 'q');
	}

	void Shutdown() override
	{

	}

private:
	Utility::Vector<GameEntity::Entity> Entities;

	uint32 AddNum = 0;
	uint32 RemovedNum = 0;
	uint32 EntitiesNum = 0;

	void CreateRandomNumEntity() 
	{
		uint32 count = rand() % 30;
		if (Entities.empty())
			count = 1000;
		Transform::InitInfo initInfo{};
		GameEntity::EntityInfo entityInfo{
			&initInfo
		};
		while (count > 0)
		{
			AddNum++;
			GameEntity::Entity entity{ GameEntity::CreateGameEntity(entityInfo) };
			assert(entity.IsValid());
			Entities.push_back(entity);
			assert(GameEntity::IsAlive(entity));
			count--;
		}
	}

	void RemoveRandomNumEntity()
	{
		uint32 count = rand() % 30;
		if (Entities.size() < 1000)
			return;
		while (count > 0)
		{
			uint32 index = (uint32)rand() % (uint32)Entities.size();
			GameEntity::Entity entity{ Entities[index] };
			assert(entity.IsValid() && Id::IsValid(entity.GetId()));
			if (entity.IsValid())
			{
				GameEntity::RemoveGameEnity(entity);
				assert(!GameEntity::IsAlive(entity));
				Entities.erase(Entities.begin() + index);
				RemovedNum++;
			}
			count--;
		}
	}

	void PrintResults()
	{
		std::cout << "エンティティ追加：" << AddNum << "\n";
		std::cout << "エンティティ削除：" << RemovedNum << "\n";
		std::cout << "エンティティ数：" << EntitiesNum << "\n";
	}
};
