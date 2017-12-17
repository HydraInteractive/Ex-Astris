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
	json["radius"] = radius;
	json["damage"] = damage;
	if (behaviour != nullptr)
	{
		json["behaviourType"] = (unsigned int)behaviour->type;
		json["pathState"] = behaviour->state;
		json["range"] = behaviour->range;
		json["originalRange"] = behaviour->originalRange;
		json["savedRange"] = behaviour->savedRange;
	}
	else
	{
		json["behaviourType"] = 0;
		json["pathState"] = 0;
		json["range"] = 0;
		json["originalRange"] = 0;
		json["savedRange"] = 0;
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
	case Behaviour::Type::BOSS_HAND:
		behaviour = std::make_shared<BossHand_Left>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::BOSS_ARMS:
		behaviour = std::make_shared<BossArm>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::STATINARY_BOSS:
		behaviour = std::make_shared<StationaryBoss>(Hydra::World::World::getEntity(entityID));
		break;
	default:
		std::cout << "Invalid AI Behaviour Type" << std::endl;
		behaviour = std::make_shared<AlienBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	}
	radius = json.value<float>("radius", 0);
	behaviour->state = json.value<int>("pathState", 0);
	damage = json.value<int>("damage", 0);

	behaviour->range = json.value<float>("range", 0);
	behaviour->originalRange = json.value<float>("originalRange", 0);
	behaviour->savedRange = json.value<float>("savedRange", 0);
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void AIComponent::registerUI() {
	ImGui::InputInt("pathState", &debugState);
	ImGui::Checkbox("isAtGoal", &behaviour->isAtGoal);
	ImGui::Checkbox("playerCanBeSeen", &behaviour->playerSeen);
	ImGui::InputFloat("range", &behaviour->range);
	ImGui::InputFloat("originalRange", &behaviour->originalRange);
}

std::shared_ptr<Hydra::World::Entity> AIComponent::getPlayerEntity()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}