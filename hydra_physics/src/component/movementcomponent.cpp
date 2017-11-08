// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* MovementComponent, used for moving.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/movementcomponent.hpp>
#include <imgui/imgui.h>

Hydra::Component::MovementComponent::~MovementComponent()
{
}

void Hydra::Component::MovementComponent::serialize(nlohmann::json & json) const
{
	json["velocityX"] = velocity[0];
	json["velocityY"] = velocity[1];
	json["velocityZ"] = velocity[2];

	json["accelerationX"] = acceleration[0];
	json["accelerationY"] = acceleration[1];
	json["accelerationZ"] = acceleration[2];

	json["movmentSpeed"] = movementSpeed;
}

void Hydra::Component::MovementComponent::deserialize(nlohmann::json & json)
{
	velocity[0] = json.value<float>("velocityX", 0);
	velocity[1] = json.value<float>("velocityY", 0);
	velocity[2] = json.value<float>("velocityZ", 0);

	acceleration[0] = json.value<float>("accelerationX", 0);
	acceleration[1] = json.value<float>("accelerationY", 0);
	acceleration[2] = json.value<float>("accelerationZ", 0);

	movementSpeed = json.value<float>("movementSpeed", 0);
}

void Hydra::Component::MovementComponent::registerUI()
{
	ImGui::DragFloat3("Direciton", glm::value_ptr(direction));
	ImGui::DragFloat3("Velocity", glm::value_ptr(velocity), 0.01f);
	ImGui::DragFloat3("Acceleration", glm::value_ptr(acceleration), 0.01f);
	ImGui::InputFloat("Movement Speed", &movementSpeed);
}
