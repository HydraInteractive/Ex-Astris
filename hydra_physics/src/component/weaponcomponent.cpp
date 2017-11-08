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
#include <hydra/component/rigidbodycomponent.hpp>
#include <btBulletDynamicsCommon.h>
#include <hydra/engine.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <random>

using namespace Hydra::World;
using namespace Hydra::Component;

using world = Hydra::World::World;

WeaponComponent::~WeaponComponent() { }

//TODO: (Re)move? to system?
void WeaponComponent::shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity) {
	if (fireRateTimer > 0)
		return;

	if (bulletSpread == 0.0f) {
		auto bullet = world::newEntity("Bullet", world::rootID);
		bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SmallCargo.mATTIC");

		auto b = bullet->addComponent<Hydra::Component::BulletComponent>();
		b->direction = direction;
		b->velocity = velocity;
		b->bulletType = bulletType;

		auto t = bullet->addComponent<Hydra::Component::TransformComponent>();
		t->position = position;
		t->scale = glm::vec3(bulletSize);
		t->rotation = bulletOrientation;

		auto bulletPhysWorld = static_cast<Hydra::System::BulletPhysicsSystem*>(IEngine::getInstance()->getState()->getPhysicsSystem());

		auto rbc = bullet->addComponent<Hydra::Component::RigidBodyComponent>();

		rbc->createBox(glm::vec3(0.5f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE, 0.0095f);
		auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
		bulletPhysWorld->enable(rbc.get());
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
		rigidBody->applyCentralForce(btVector3(b->direction.x, b->direction.y, b->direction.z) * 300);
		rigidBody->setGravity(btVector3(0, 0, 0));
	} else {
		for (int i = 0; i < bulletsPerShot; i++) {
			auto bullet = world::newEntity("Bullet", world::rootID);
			bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SmallCargo.mATTIC");

			float phi = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
			float distance = ((float)rand() / (float)(RAND_MAX)) * bulletSpread;
			float theta = ((float)rand() / (float)(RAND_MAX)) * 3.14f;

			glm::vec3 bulletDirection = direction;
			bulletDirection.x += distance * sin(theta) * cos(phi);
			bulletDirection.y += distance * sin(theta) * sin(phi);
			bulletDirection.z += distance * cos(theta);
			bulletDirection = glm::normalize(bulletDirection);

			auto b = bullet->addComponent<Hydra::Component::BulletComponent>();
			b->direction = bulletDirection;
			b->velocity = velocity;
			b->bulletType = bulletType;

			auto t = bullet->addComponent<Hydra::Component::TransformComponent>();
			t->position = position;
			t->scale = glm::vec3(bulletSize);
			t->rotation = bulletOrientation;

			auto bulletPhysWorld = static_cast<Hydra::System::BulletPhysicsSystem*>(IEngine::getInstance()->getState()->getPhysicsSystem());

			auto rbc = bullet->addComponent<Hydra::Component::RigidBodyComponent>();
			rbc->createBox(glm::vec3(0.5f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE, 0.0095f);
			auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
			bulletPhysWorld->enable(rbc.get());
			rigidBody->setActivationState(DISABLE_DEACTIVATION);
			rigidBody->applyCentralForce(btVector3(b->direction.x, b->direction.y, b->direction.z) * 300);
			rigidBody->setGravity(btVector3(0,0,0));

		}
	}
	fireRateTimer = 1.0f/(fireRateRPM / 60.0f);

}

void WeaponComponent::serialize(nlohmann::json& json) const {
	json["fireRateRPM"] = fireRateRPM;
	json["ignoreParent"] = bulletSize;
	json["ignoreParent"] = bulletSpread;
	json["ignoreParent"] = bulletsPerShot;
}

void WeaponComponent::deserialize(nlohmann::json& json) {
	fireRateRPM = json.value<int>("fireRateRPM", 0);
	bulletSize = json.value<float>("bulletSize", 0);
	bulletSpread = json.value<float>("bulletSpread", 0);
	bulletsPerShot = json.value<int>("bulletsPerShot", 0);
}

void WeaponComponent::registerUI() {
	ImGui::DragFloat("Fire Rate RPM", &fireRateRPM);
	ImGui::DragFloat("Bullet Size", &bulletSize, 0.001f);
	ImGui::DragFloat("Bullet Spread", &bulletSpread, 0.001f);
	ImGui::InputInt("Bullets Per Shot", &bulletsPerShot);
}
