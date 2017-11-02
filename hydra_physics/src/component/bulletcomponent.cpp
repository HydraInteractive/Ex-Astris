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
		{ "position", { position.x, position.y, position.z } },
		{ "direction", { direction.x, direction.y, direction.z } },
		{ "velocity", velocity }
	};
}

void BulletComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& dir = json["direction"];
	direction = glm::vec3{ dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>() };

	velocity = json["velocity"].get<float>();
}

void BulletComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(position));
	ImGui::DragFloat3("Direction", glm::value_ptr(direction));
	ImGui::DragFloat("Velocity", &velocity);
}
