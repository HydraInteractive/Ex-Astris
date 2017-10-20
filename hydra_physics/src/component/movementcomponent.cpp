// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* MovementComponent, used for moving.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include "hydra\component\movementcomponent.hpp"
#include <imgui/imgui.h>

Hydra::Component::MovementComponent::~MovementComponent()
{
}

void Hydra::Component::MovementComponent::serialize(nlohmann::json & json) const
{
	json = {
		{ "velocity",{ velocity.x, velocity.y, velocity.z } },
		{ "acceleration",{ acceleration.x, acceleration.y, acceleration.z } },
		{ "movementSpeed", movementSpeed },
	};
}

void Hydra::Component::MovementComponent::deserialize(nlohmann::json & json)
{
	auto& velo = json["velocity"];
	velocity = glm::vec3{ velo[0].get<float>(), velo[1].get<float>(), velo[2].get<float>() };
	auto& acce = json["acceleration"];
	acceleration = glm::vec3{ acce[0].get<float>(), acce[1].get<float>(), acce[2].get<float>() };
	movementSpeed = json["movementSpeed"].get<float>();
}

void Hydra::Component::MovementComponent::registerUI()
{
	ImGui::DragFloat3("Velocity", glm::value_ptr(velocity), 0.01f);
	ImGui::DragFloat3("Acceleration", glm::value_ptr(acceleration), 0.01f);
	ImGui::InputFloat("Movement Speed", &movementSpeed);
}
