// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/playercomponent.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

PlayerComponent::~PlayerComponent() {}


std::shared_ptr<Hydra::World::Entity> PlayerComponent::getWeapon() {
	auto& children = Hydra::World::World::getEntity(entityID)->children;

	for (auto child : children)
		if (auto c =Hydra::World::World::getEntity(child); c->name == "Weapon")
			return c;

	return std::shared_ptr<Hydra::World::Entity>();
}
void PlayerComponent::serialize(nlohmann::json& json) const {
	json["weaponOffsetX"] = weaponOffset[0];
	json["weaponOffsetY"] = weaponOffset[1];
	json["weaponOffsetZ"] = weaponOffset[2];
	
	json["onGround"] = onGround;
	json["firstPerson"] = firstPerson;
	json["isDead"] = isDead;
}

void PlayerComponent::deserialize(nlohmann::json& json) {
	weaponOffset[0] = json.value<float>("weaponOffsetX", 0);
	weaponOffset[1] = json.value<float>("weaponOffsetY", 0);
	weaponOffset[2] = json.value<float>("weaponOffsetZ", 0);

	onGround = json.value<bool>("onGround", 0);
	firstPerson = json.value<bool>("firstPerson", 0);
	isDead = json.value<bool>("isDead", 0);
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::DragFloat3("Weapon Offset", glm::value_ptr(weaponOffset),0.01f);
	ImGui::Checkbox("On Ground", &onGround);
	ImGui::Checkbox("First Person", &firstPerson);
	ImGui::Checkbox("Is Dead", &isDead);
}
