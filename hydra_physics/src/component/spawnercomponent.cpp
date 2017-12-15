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

	json["playerPosX"] = playerPos[0];
	json["playerPosY"] = playerPos[1];
	json["playerPosZ"] = playerPos[2];
}

void Hydra::Component::SpawnerComponent::deserialize(nlohmann::json & json)
{
	//spawnerID = json.value<int>("spawnerID", 0);
	spawnerID = (SpawnerType)json["spawnerID"].get<int>();
	spawnCounter = json["spawnCounter"].get<int>();

	playerPos[0] = json.value<float>("playerPosX", 0);
	playerPos[1] = json.value<float>("playerPosY", 0);
	playerPos[2] = json.value<float>("playerPosZ", 0);
}

void Hydra::Component::SpawnerComponent::registerUI()
{
}

void Hydra::Component::SpawnerComponent::setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player)
{
	playerPos = player->getComponent<Hydra::Component::TransformComponent>().get()->position;
}