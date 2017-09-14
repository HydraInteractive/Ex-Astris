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

using namespace Hydra::World;
using namespace Hydra::Component;

PlayerComponent::PlayerComponent(IEntity* entity) : IComponent(entity) {
	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;
}

PlayerComponent::~PlayerComponent() { }

void PlayerComponent::tick(TickAction action) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	// Extract players position
	auto player = entity->getComponent<Component::TransformComponent>();
	// Extract cameras position
	auto camera = entity->getComponent<Component::CameraComponent>();
	
	{
		Uint8* keysArray;
		keysArray = const_cast <Uint8*> (SDL_GetKeyboardState(NULL));

		if (keysArray[SDL_SCANCODE_UP]) {
			_velocityZ = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_DOWN]) {
			_velocityZ = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_LEFT]) {
			_velocityX = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_RIGHT]) {
			_velocityX = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_LEFT] == 0 && keysArray[SDL_SCANCODE_RIGHT] == 0) {
			_velocityX = 0.0f;
		}

		if (keysArray[SDL_SCANCODE_UP] == 0 && keysArray[SDL_SCANCODE_DOWN] == 0) {
			_velocityZ = 0.0f;
		}
	}

	glm::mat4 viewMat = camera->getViewMatrix();
	glm::vec3 forward(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
	glm::vec3 strafe(viewMat[0][0], viewMat[1][0], viewMat[2][0]);

	glm::vec3 movementVector = (_velocityZ * forward + _velocityX * strafe);
	movementVector.y = 0.0f;

	_playerPos += movementVector;

	camera->setPosition(_playerPos);
	player->setPosition(_playerPos + glm::vec3(0, 3, 0) + (forward * glm::vec3(-4, 0, -4)));
	player->setRotation(glm::angleAxis(-camera->getYaw(), glm::vec3(0, 1, 0)));
}

void PlayerComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _playerPos.x, _playerPos.y, _playerPos.z } },
		{ "velocityX", _velocityX },
		{ "velocityY", _velocityY },
		{ "velocityZ", _velocityZ },
	};
}

void PlayerComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_playerPos = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	_velocityX = json["velocityX"].get<float>();
	_velocityY = json["velocityY"].get<float>();
	_velocityZ = json["velocityZ"].get<float>();
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::InputFloat("X", &_playerPos.x);
	ImGui::InputFloat("Y", &_playerPos.y);
	ImGui::InputFloat("Z", &_playerPos.z);
	ImGui::InputFloat("DEBUG", &_debug);
}