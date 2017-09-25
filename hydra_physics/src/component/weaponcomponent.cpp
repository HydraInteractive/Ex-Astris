// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Weapon stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/weaponcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <random>

using namespace Hydra::World;
using namespace Hydra::Component;

WeaponComponent::WeaponComponent(IEntity* entity) : IComponent(entity) {

}

WeaponComponent::~WeaponComponent() { }

void WeaponComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

}

void WeaponComponent::shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity)
{
	if (SDL_GetTicks() > _fireRateTimer + 1000/(_fireRateRPM/60)){
		if (_bulletSpread == 0.0f){
			auto bullet = getBullets()->createEntity("Bullet");
			
			bullet->addComponent<Hydra::Component::MeshComponent>("assets/objects/Fridge.ATTIC");
			bullet->addComponent<Hydra::Component::BulletComponent>(position, -direction, 2.0f);
			auto transform = bullet->addComponent<Hydra::Component::TransformComponent>(position, glm::vec3(_bulletSize));

			transform->setRotation(bulletOrientation);
		}
		else {
			for (int i = 0; i < _bulletsPerShot; i++) {
				auto bullet = getBullets()->createEntity("Bullet");
				bullet->addComponent<Hydra::Component::MeshComponent>("assets/objects/Fridge.ATTIC");

				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<>dis(0, 2 * 3.14);
				float phi = dis(gen);
				dis = std::uniform_real_distribution<>(0, 1.0);
				float distance = dis(gen) * _bulletSpread;
				dis = std::uniform_real_distribution<>(0, 3.14);
				float theta = dis(gen);

				glm::vec3 bulletDirection = -direction;
				bulletDirection.x += distance * sin(theta) * cos(phi);
				bulletDirection.y += distance * sin(theta) * sin(phi);
				bulletDirection.z += distance * cos(theta);
				bulletDirection = glm::normalize(bulletDirection);

				bullet->addComponent<Hydra::Component::BulletComponent>(position, bulletDirection, 2.0f);

				auto transform = bullet->addComponent<Hydra::Component::TransformComponent>(position, glm::vec3(_bulletSize));
				transform->setRotation(bulletOrientation);
			}
		}
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
		{ "fireRateRPM", _fireRateRPM },
		{ "bulletSize", _bulletSize},
		{ "bulletSpread", _bulletSpread},
		{ "bulletsPerShot", _bulletsPerShot}
	};
}

void WeaponComponent::deserialize(nlohmann::json& json) {
	_fireRateRPM = json["fireRateRPM"].get<int>();
	_bulletSize = json["bulletSize"].get<float>();
	_bulletSpread = json["bulletSpread"].get<float>();
	_bulletsPerShot = json["bulletsPerShot"].get<int>();
}

void WeaponComponent::registerUI() {
	ImGui::InputInt("Fire Rate RPM", &_fireRateRPM);
	ImGui::DragFloat("Bullet Size", &_bulletSize, 0.001f);
	ImGui::DragFloat("Bullet Spread", &_bulletSpread, 0.001f);
	ImGui::InputInt("Bullets Per Shot", &_bulletsPerShot);
	ImGui::DragFloat3("DEBUG", glm::value_ptr(_debug));
}
