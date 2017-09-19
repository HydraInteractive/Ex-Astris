// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/weaponcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

WeaponComponent::WeaponComponent(IEntity* entity) : IComponent(entity) {

}
WeaponComponent::WeaponComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction, float velocity) : IComponent(entity) {

}

WeaponComponent::~WeaponComponent() { }

void WeaponComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

}

void WeaponComponent::shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity)
{
	if (SDL_GetTicks() > _fireRateTimer + 50)
	{
		std::shared_ptr<Hydra::World::IEntity> bullet = getBullets()->createEntity("Bullet");
		bullet->addComponent<Hydra::Component::MeshComponent>("assets/objects/alphaGunModel.ATTIC");
		bullet->addComponent<Hydra::Component::BulletComponent>(position, -direction, 0.1f);
		auto transform = bullet->addComponent<Hydra::Component::TransformComponent>(position);
		transform->setRotation(bulletOrientation);
		_fireRateTimer = SDL_GetTicks();
	}
}

std::shared_ptr<Hydra::World::IEntity> WeaponComponent::getBullets() {
	std::shared_ptr<Hydra::World::IEntity> bullets;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children = entity->getChildren();

	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->getName() == "Bullets") {
			bullets = children[i];
		}
	}

	return bullets;
}

void WeaponComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "firerateTimer", _fireRateTimer }
	};
}

void WeaponComponent::deserialize(nlohmann::json& json) {
	_fireRateTimer = json["firerateTimer"].get<unsigned int>();
}

void WeaponComponent::registerUI() {

}
