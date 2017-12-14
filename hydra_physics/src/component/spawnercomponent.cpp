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
		{ "spawnCounter", (int)spawnCounter },
	};
}

void Hydra::Component::SpawnerComponent::deserialize(nlohmann::json & json)
{
	//spawnerID = json.value<int>("spawnerID", 0);
	spawnerID = (SpawnerType)json["spawnerID"].get<int>();
	spawnCounter = json["spawnCounter"].get<int>();
}

void Hydra::Component::SpawnerComponent::registerUI()
{
}

std::shared_ptr<Hydra::World::Entity> SpawnerComponent::getPlayerEntity()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}