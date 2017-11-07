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
	json = {
		{ "direction", { direction.x, direction.y, direction.z } },
		{ "velocity", velocity },
		{ "damage", damage }
	};
}

void BulletComponent::deserialize(nlohmann::json& json) {
	auto& dir = json["direction"];
	direction = glm::vec3{ dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>() };

	velocity = json["velocity"].get<float>();
	damage = json.value<float>("damage", 1.0f);
}

void BulletComponent::registerUI() {
	ImGui::DragFloat3("Direction", glm::value_ptr(direction));
	ImGui::DragFloat("Velocity", &velocity);
}
