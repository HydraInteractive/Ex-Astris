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
	entity->createEntity("Grenades");
}

PlayerComponent::~PlayerComponent() { }

void PlayerComponent::tick(TickAction action, float delta) {
	auto player = entity->getComponent<Component::TransformComponent>();

	auto camera = entity->getComponent<Component::CameraComponent>();

	auto weapon = getWeapon()->getComponent<Component::WeaponComponent>();
	
	glm::mat4 viewMat = camera->getViewMatrix();
	glm::vec3 forward(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
	glm::vec3 strafe(viewMat[0][0], viewMat[1][0], viewMat[2][0]);

	{
		Uint8* keysArray;
		keysArray = const_cast <Uint8*> (SDL_GetKeyboardState(NULL));

		if (keysArray[SDL_SCANCODE_W]) {
			_velocity.z = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_S]) {
			_velocity.z = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_A]) {
			_velocity.x = -_movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_D]) {
			_velocity.x = _movementSpeed;
		}

		if (keysArray[SDL_SCANCODE_A] == 0 && keysArray[SDL_SCANCODE_D] == 0) {
			_velocity.x = 0.0f;
		}

		if (keysArray[SDL_SCANCODE_W] == 0 && keysArray[SDL_SCANCODE_S] == 0) {
			_velocity.z = 0.0f;
		}

		if (keysArray[SDL_SCANCODE_SPACE] && _onGround){
			_acceleration.y -= 6.0f;
			_onGround = false;
		}
		if (keysArray[SDL_SCANCODE_F]) {

		}
		if (keysArray[SDL_SCANCODE_G]){

			std::shared_ptr<Hydra::World::IEntity> grenades;
			std::vector<std::shared_ptr<Hydra::World::IEntity>> children = entity->getChildren();

			for (size_t i = 0; i < children.size(); i++) {
				if (children[i]->getName() == "Grenades") {
					grenades = children[i];
				}
			}

			auto grenade = grenades->createEntity("grenade");
			grenade->addComponent<Component::GrenadeComponent>(_position, -forward);
		}

		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
			glm::quat bulletOrientation = glm::angleAxis(-camera->getYaw(), glm::vec3(0, 1, 0)) * (glm::angleAxis(-camera->getPitch(), glm::vec3(1, 0, 0)));
			weapon->shoot(_position, forward, bulletOrientation, 0.5f);
		}
	}

	_acceleration.y += 10.0f * delta;

	glm::vec3 movementVector = (_velocity.z * forward + _velocity.x * strafe);
	movementVector.y = _acceleration.y;
	_debug = _acceleration.y;

	_position += movementVector * delta;

	
	if (_position.y > 0) {
		_position.y = 0;
		_acceleration.y = 0;
		_onGround = true;
	}

	if (_firstPerson){
		camera->setPosition(_position);
	}
	else{
		camera->setPosition(_position + glm::vec3(0, -3, 0) + (forward * glm::vec3(4, 0, 4)));
	}
	player->setPosition(_position);
	
	player->setRotation(glm::angleAxis(camera->getYaw(), glm::vec3(0, 1, 0)));
	
	player->setPosition(_position + glm::vec3(0, 0.75, 0) + glm::vec3(-1, 0, -1) * forward + glm::vec3(1, 0, 1)*strafe);
	_debugPos = forward*glm::vec3(-2, 0, -2);
}

std::shared_ptr<Hydra::World::IEntity> PlayerComponent::getWeapon() {
	std::shared_ptr<Hydra::World::IEntity> weapon;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children = entity->getChildren();

	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->getName() == "Weapon") {
			weapon = children[i];
			i = children.size();
		}
	}

	return weapon;
}

void PlayerComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "velocity",{ _velocity.x, _velocity.y, _velocity.z } }
	};
}

void PlayerComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };
	
	auto& vel = json["velocity"];
	_position = glm::vec3{ vel[0].get<float>(), vel[1].get<float>(), vel[2].get<float>() };
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position),0.01f);
	ImGui::InputFloat("DEBUG", &_debug);
	ImGui::DragFloat3("DEBUG POS", glm::value_ptr(_debugPos), 0.01f);
	ImGui::Checkbox("First Person", &_firstPerson);
}
