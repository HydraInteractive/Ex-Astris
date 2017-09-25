// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Bullet stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/grenadecomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

GrenadeComponent::GrenadeComponent(IEntity* entity) : IComponent(entity) {

}

GrenadeComponent::GrenadeComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction) : IComponent(entity) {
	_position = position;
	_direction = direction;
	_velocity = 20.0f;
	entity->addComponent<Component::MeshComponent>("assets/objects/Fridge.ATTIC");
	entity->addComponent<Component::TransformComponent>(_position);
}


GrenadeComponent::~GrenadeComponent() { }

void GrenadeComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.
	//_direction -= 0.1f * delta;
	_direction.y += 0.8f * delta;
	_position += (_direction * _velocity) * delta;
	if (_position.y > 0.0f) {
		_position.y = 0.0f;
	}
	auto transform = entity->getComponent<Component::TransformComponent>();
	transform->setPosition(_position);
}

void GrenadeComponent::serialize(nlohmann::json& json) const {

}

void GrenadeComponent::deserialize(nlohmann::json& json) {
	
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void GrenadeComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position));
	ImGui::DragFloat3("Direction", glm::value_ptr(_direction));
	ImGui::DragFloat("Velocity", &_velocity);
}
