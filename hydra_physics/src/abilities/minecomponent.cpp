// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Bullet stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/abilities/minecomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

MineComponent::MineComponent(IEntity* entity) : IComponent(entity) {

}

MineComponent::MineComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction) : IComponent(entity) {
	_position = position;
	_direction = direction;
	_velocity = 20.0f;
	_fallingVelocity = 20.0f;
	_tickSpawned = SDL_GetTicks();
	_timeUntilDelete = 5*1000;
	entity->addComponent<Component::MeshComponent>("assets/objects/Fridge.ATTIC");
	entity->addComponent<Component::TransformComponent>(_position);
}


MineComponent::~MineComponent() { }

void MineComponent::tick(TickAction action, float delta) {

	_direction.y += 0.8f * delta;

	_velocity -= 2.0f * delta;

	if (_velocity < 0) {
		_velocity = 0;
	}
	if (_fallingVelocity < 0) {
		_fallingVelocity = 0;
	}

	glm::vec3 temp(_velocity, _fallingVelocity, _velocity);

	_position += _direction * temp * delta;

	if (_position.y > 0.0f) {
		_position.y = 0.0f;
		_velocity = 0.0f;
		_fallingVelocity = 0.0f;
	}

	if (SDL_GetTicks() > _tickSpawned + _timeUntilDelete)
	{
		entity->markDead();
	}

	auto transform = entity->getComponent<Component::TransformComponent>();
	transform->setPosition(_position);
}

void MineComponent::serialize(nlohmann::json& json) const {

}

void MineComponent::deserialize(nlohmann::json& json) {

}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void MineComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position));
}
