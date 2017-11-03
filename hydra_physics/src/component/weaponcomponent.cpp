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

using world = Hydra::World::World;

WeaponComponent::~WeaponComponent() { }

//TODO: (Re)move? to system?
void WeaponComponent::shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity) {
	if (!_bullets)
		_bullets = world::newEntity("Bullets", entityID);

	if (fireRateTimer > 0)
		return;

	if (bulletSpread == 0.0f) {
		auto bullet = world::newEntity("Bullet", _bullets);
		bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Fridge.ATTIC");
		auto b = bullet->addComponent<Hydra::Component::BulletComponent>();
		b->position = position;
		b->direction = -direction;
		b->velocity = velocity;
		auto transform = bullet->addComponent<Hydra::Component::TransformComponent>();
		transform->position = position;
		transform->scale = glm::vec3(bulletSize);
		transform->rotation = bulletOrientation;
	} else {
		for (int i = 0; i < bulletsPerShot; i++) {
			auto bullet = world::newEntity("Bullet", _bullets);
			bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Fridge.ATTIC");

			std::random_device rd; //
			std::mt19937 gen(rd()); // FIXME: Why always the same random?
			std::uniform_real_distribution<>dis(0, 2 * 3.14); //
			float phi = dis(gen);
			dis = std::uniform_real_distribution<>(0, 1.0);
			float distance = dis(gen) * bulletSpread;
			dis = std::uniform_real_distribution<>(0, 3.14);
			float theta = dis(gen);

			glm::vec3 bulletDirection = -direction;
			bulletDirection.x += distance * sin(theta) * cos(phi);
			bulletDirection.y += distance * sin(theta) * sin(phi);
			bulletDirection.z += distance * cos(theta);
			bulletDirection = glm::normalize(bulletDirection);

			auto b = bullet->addComponent<Hydra::Component::BulletComponent>();
			b->position = position;
			b->direction = bulletDirection;
			b->velocity = velocity;

			auto transform = bullet->addComponent<Hydra::Component::TransformComponent>();
			transform->position = position;
			transform->scale = glm::vec3(bulletSize);
			transform->rotation = bulletOrientation;
		}
	}
	fireRateTimer = fireRateRPM / 60000.0;

}

void WeaponComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "fireRateRPM", fireRateRPM },
		{ "bulletSize", bulletSize},
		{ "bulletSpread", bulletSpread},
		{ "bulletsPerShot", bulletsPerShot}
	};
}

void WeaponComponent::deserialize(nlohmann::json& json) {
	fireRateRPM = json["fireRateRPM"].get<int>();
	bulletSize = json["bulletSize"].get<float>();
	bulletSpread = json["bulletSpread"].get<float>();
	bulletsPerShot = json["bulletsPerShot"].get<int>();
}

void WeaponComponent::registerUI() {
	ImGui::DragFloat("Fire Rate RPM", &fireRateRPM);
	ImGui::DragFloat("Bullet Size", &bulletSize, 0.001f);
	ImGui::DragFloat("Bullet Spread", &bulletSpread, 0.001f);
	ImGui::InputInt("Bullets Per Shot", &bulletsPerShot);
}
