// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Bullet stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/bulletcomponent.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

BulletComponent::~BulletComponent() { }

void BulletComponent::serialize(nlohmann::json& json) const {
	json["directionX"] = direction[0];
	json["directionY"] = direction[1];
	json["directionZ"] = direction[2];

	json["velocity"] = velocity;
	json["damage"] = damage;
}

void BulletComponent::deserialize(nlohmann::json& json) {
	direction[0] = json.value<float>("directionX", 0);
	direction[1] = json.value<float>("directionY", 0);
	direction[2] = json.value<float>("directionZ", 0);

	velocity = json.value<float>("velocity", 0);
	damage = json.value<float>("damage", 1.0f);
}

void BulletComponent::registerUI() {
	ImGui::DragFloat3("Direction", glm::value_ptr(direction));
	ImGui::DragFloat("Velocity", &velocity);
}
