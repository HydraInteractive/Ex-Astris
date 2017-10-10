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

BulletComponent::BulletComponent(IEntity* entity) : IComponent(entity) {

}
BulletComponent::BulletComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction, float velocity) : IComponent(entity) {
	_position = position;
	_direction = direction;
	_velocity = velocity;
	_deleteTimer = SDL_GetTicks();
}

BulletComponent::~BulletComponent() { }

void BulletComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.
	_position += _velocity * _direction * delta;
	
	auto transform = entity->getComponent<Component::TransformComponent>();
	transform->setPosition(_position);
	//if (SDL_GetTicks() > _deleteTimer + 4*1000){
	//	entity->markDead();
	//}
}

void BulletComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "direction",{ _direction.x, _direction.y, _direction.z } },
		{ "velocity", _velocity }
	};
}

void BulletComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& dir = json["direction"];
	_direction = glm::vec3{ dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>() };

	_velocity = json["velocity"].get<float>();
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void BulletComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position));
	ImGui::DragFloat3("Direction", glm::value_ptr(_direction));
	ImGui::DragFloat("Velocity", &_velocity);
}
