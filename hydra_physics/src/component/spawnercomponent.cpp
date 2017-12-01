// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Spawner stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include "hydra/component/spawnercomponent.hpp"
#include "hydra/component/roomcomponent.hpp"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

Hydra::Component::SpawnerComponent::~SpawnerComponent() {}

void Hydra::Component::SpawnerComponent::serialize(nlohmann::json & json) const
{
	//json["spawnerID"] = spawnerID;
	json = {
		{ "spawnerID", (int)spawnerID },
	};

	json["mapSize"] = (unsigned int)ROOM_MAP_SIZE;
	for (size_t i = 0; i < WORLD_MAP_SIZE; i++)
	{
		for (size_t j = 0; j < WORLD_MAP_SIZE; j++)
		{
			json["map"][i][j] = map[i][j];
		}
	}
}

void Hydra::Component::SpawnerComponent::deserialize(nlohmann::json & json)
{
	//spawnerID = json.value<int>("spawnerID", 0);
	spawnerID = (SpawnerType)json["spawnerID"].get<int>();

	if (json.value<unsigned int>("mapSize", 0) == ROOM_MAP_SIZE)
	{
		for (size_t i = 0; i < ROOM_MAP_SIZE; i++)
		{
			for (size_t j = 0; j < ROOM_MAP_SIZE; j++)
			{
				map[i][j] = json["map"][i][j].get<bool>();
			}
		}
	}
	else
	{
		for (size_t i = 0; i < ROOM_MAP_SIZE; i++)
		{
			for (size_t j = 0; j < ROOM_MAP_SIZE; j++)
			{
				map[i][j] = true;
			}
		}
	}
}

void Hydra::Component::SpawnerComponent::registerUI()
{
}

std::shared_ptr<Hydra::World::Entity> SpawnerComponent::getPlayerEntity()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}