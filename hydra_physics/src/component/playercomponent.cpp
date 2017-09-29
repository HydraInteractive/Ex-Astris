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

PlayerComponent::PlayerComponent(IEntity* entity) : IComponent(entity) {
	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;
	_position = glm::vec3(0, -2, 20);
	_health = 100;
	_timer = SDL_GetTicks();
}

PlayerComponent::~PlayerComponent() { }


void PlayerComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	if (_health <= 0)
	{
		_dead = true;
	}

	// Extract players position
	auto player = entity->getComponent<Component::TransformComponent>();
	// Extract cameras position
	auto camera = entity->getComponent<Component::CameraComponent>();
	
	{
		Uint8* keysArray;
		keysArray = const_cast <Uint8*> (SDL_GetKeyboardState(NULL));

		if (keysArray[SDL_SCANCODE_W]) {
			_velocityZ = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_S]) {
			_velocityZ = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_A]) {
			_velocityX = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_D]) {
			_velocityX = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_A] == 0 && keysArray[SDL_SCANCODE_D] == 0) {
			_velocityX = 0.0f;
		}

		if (keysArray[SDL_SCANCODE_W] == 0 && keysArray[SDL_SCANCODE_S] == 0) {
			_velocityZ = 0.0f;
		}

		if (keysArray[SDL_SCANCODE_SPACE] && _onGround){
			_accelerationY -= 0.3f;
			_onGround = false;
		}
	}

	_accelerationY += 0.01f;

	glm::mat4 viewMat = camera->getViewMatrix();
	glm::vec3 forward(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
	glm::vec3 strafe(viewMat[0][0], viewMat[1][0], viewMat[2][0]);

	glm::vec3 movementVector = (_velocityZ * forward + _velocityX * strafe);
	movementVector.y = _accelerationY;
	_debug = _accelerationY;

	_position += movementVector;

	
	if (_position.y > 0) {
		_position.y = 0;
		_accelerationY = 0;
		_onGround = true;
	}

	if (_firstPerson){
		camera->setPosition(_position);
	}
	else{
		camera->setPosition(_position + glm::vec3(0, -3, 0) + (forward * glm::vec3(4, 0, 4)));
	}
	player->setPosition(_position);
	//player->setRotation(glm::angleAxis(-camera->getYaw(), glm::vec3(0, 1, 0)) * (glm::angleAxis(-camera->getPitch(), glm::vec3(1, 0, 0))));
	player->setRotation(glm::angleAxis(camera->getYaw(), glm::vec3(0, 1, 0)));
	//player->setRotation(glm::angleAxis(-camera->getPitch(), glm::vec3(1, 0, 0)));
	player->setPosition(_position + glm::vec3(0, 0.75, 0) + glm::vec3(-1, 0, -1) * forward + glm::vec3(1, 0, 1)*strafe);
	_debugPos = forward*glm::vec3(-2, 0, -2);
}

glm::vec3 PlayerComponent::getPosition()
{
	return _position;
}

int Hydra::Component::PlayerComponent::getHealth()
{
	return _health;
}

void Hydra::Component::PlayerComponent::applyDamage(int damage)
{
	if (SDL_GetTicks() > _timer + 500)
	{
		_health = _health - damage;
		_timer = SDL_GetTicks();
	}
}

void PlayerComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "velocityX", _velocityX },
		{ "velocityY", _velocityY },
		{ "velocityZ", _velocityZ },
	};
}

void PlayerComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	_velocityX = json["velocityX"].get<float>();
	_velocityY = json["velocityY"].get<float>();
	_velocityZ = json["velocityZ"].get<float>();
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position),0.01f);
	ImGui::InputFloat("DEBUG", &_debug);
	ImGui::DragFloat3("DEBUG POS", glm::value_ptr(_debugPos), 0.01f);
	ImGui::Checkbox("First Person", &_firstPerson);
	ImGui::Checkbox("Dead", &_dead);
}
