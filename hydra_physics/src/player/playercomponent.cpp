// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <player/playercomponent.hpp>

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
	
	Uint8* keysArray;
	keysArray = const_cast <Uint8*> (SDL_GetKeyboardState(NULL));

	if (keysArray[SDL_SCANCODE_UP]) {
		_velocityZ = -0.5f;
	}
	if (keysArray[SDL_SCANCODE_DOWN]) {
		_velocityZ = 0.5f;
	}
	if (keysArray[SDL_SCANCODE_LEFT]) {
		_velocityX = -0.5f;
	}
	if (keysArray[SDL_SCANCODE_RIGHT]) {
		_velocityX = 0.5f;
	}
	
	if (keysArray[SDL_SCANCODE_LEFT] == 0 && keysArray[SDL_SCANCODE_RIGHT] == 0) {
		_velocityX = 0.0f;
	}

	if (keysArray[SDL_SCANCODE_UP] == 0 && keysArray[SDL_SCANCODE_DOWN] == 0) {
		_velocityZ = 0.0f;
	}

	if (keysArray[SDL_SCANCODE_SPACE] == 0)
	{
		//_velocityY = 0.1f;
	}
	if (keysArray[SDL_SCANCODE_SPACE] == 1)
	{
		//_velocityY = -0.1f;
	}

	_playerPos = camera->getPosition();
	_playerPos = _playerPos + glm::vec3(_velocityX, _velocityY, _velocityZ);

	player->setPosition(glm::vec3(_playerPos.x, _playerPos.y+2, _playerPos.z-4));
	camera->setPosition(_playerPos);


}

msgpack::packer<msgpack::sbuffer>& PlayerComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	/*
	* // Optional code, not really needed
	*
	* o.pack_map(3); // The amount of variables to be written
	*
	* // The variables:
	* o.pack("a");
	* o.pack(_a);
	*
	* o.pack("b");
	* o.pack(_b);
	*
	* o.pack("c");
	* o.pack(_c);
	*/
	return o;
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::InputFloat("X", &_playerPos.x);
	ImGui::InputFloat("Y", &_playerPos.y);
	ImGui::InputFloat("Z", &_playerPos.z);
}
