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
	velocityX = 0;
	velocityY = 0;
	velocityZ = 0;
}

PlayerComponent::~PlayerComponent() { }

void PlayerComponent::tick(TickAction action) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	// Extract players position
	//auto player = entity->getComponent<Component::TransformComponent>();
	auto camera = entity->getComponent<Component::CameraComponent>();

	playerPos = camera->getPosition();
	
	Uint8* keysArray;
	keysArray = const_cast <Uint8*> (SDL_GetKeyboardState(NULL));

	if (keysArray[SDL_SCANCODE_UP]) {
		velocityZ = 0.5f;
	}
	if (keysArray[SDL_SCANCODE_DOWN]) {
		velocityZ = -0.5f;
	}
	if (keysArray[SDL_SCANCODE_LEFT]) {
		velocityX = 0.5f;
	}
	if (keysArray[SDL_SCANCODE_RIGHT]) {
		velocityX = -0.5f;
	}
	
	if (keysArray[SDL_SCANCODE_LEFT] == 0 && keysArray[SDL_SCANCODE_RIGHT] == 0) {
		velocityX = 0.0f;
	}

	if (keysArray[SDL_SCANCODE_UP] == 0 && keysArray[SDL_SCANCODE_DOWN] == 0) {
		velocityZ = 0.0f;
	}

	if (keysArray[SDL_SCANCODE_SPACE] == 0)
	{
		//velocityY = 0.1f;
	}
	if (keysArray[SDL_SCANCODE_SPACE] == 1)
	{
		//velocityY = -0.1f;
	}

	playerPos = playerPos + glm::vec3(velocityX, velocityY, velocityZ);

	camera->setPosition(playerPos);


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

	
	ImGui::InputFloat("X", &playerPos.x);
	ImGui::InputInt("A", &_a);
	ImGui::Checkbox("B", &_b);
	ImGui::DragFloat("C", &_c);
}
