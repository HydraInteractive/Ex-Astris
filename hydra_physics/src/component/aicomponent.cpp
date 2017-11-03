// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* AIComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#include <hydra/component/aicomponent.hpp>
#include <imgui/imgui.h>
#include <hydra/component/drawobjectcomponent.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

AIComponent::~AIComponent() {
	
}

void AIComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "behaviourType", (unsigned int)behaviour->type},
		{ "radius",{ radius } },
		{ "pathState", behaviour->state },
		{ "damage", damage },
		{ "range", behaviour->range },
		{ "original range", behaviour->originalRange }
	};

	for (size_t i = 0; i < MAP_SIZE; i++)
	{
		for (size_t j = 0; j < MAP_SIZE; j++)
		{
			json["map"][i][j] = behaviour->map[i][j];
		}
	}
}

void AIComponent::deserialize(nlohmann::json& json) {
		Behaviour::Type behaviourType = (Behaviour::Type)json["behaviourType"].get<unsigned int>();
	switch (behaviourType)
	{
	case Behaviour::Type::ALIEN:
		behaviour = std::make_shared<AlienBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::ROBOT:
		behaviour = std::make_shared<RobotBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::ALIENBOSS:
		behaviour = std::make_shared<AlienBossBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	default:
		std::cout << "Invalid AI Behaviour Type" << std::endl;
		break;
	}
	radius = json["radius"].get<float>();

	behaviour->state = json["pathState"].get<int>();
	damage = json["damage"].get<int>();


	behaviour->range = json["range"].get<float>();
	behaviour->originalRange = json["original range"].get<float>();

	for (size_t i = 0; i < MAP_SIZE; i++)
	{
		for (size_t j = 0; j < MAP_SIZE; j++)
		{
			behaviour->map[i][j] = json["map"][i][j].get<int>();
		}
	}
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void AIComponent::registerUI() {
	ImGui::InputInt("pathState", &debugState);
	ImGui::Checkbox("isAtGoal", &behaviour->isAtGoal);
	ImGui::Checkbox("playerCanBeSeen", &behaviour->playerSeen);
	ImGui::InputFloat("range", &behaviour->range);
}

std::shared_ptr<Hydra::World::Entity> AIComponent::getPlayerEntity()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}

