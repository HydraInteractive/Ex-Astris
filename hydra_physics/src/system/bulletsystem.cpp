#include <hydra/system/bulletsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/aicomponent.hpp>

#include <btBulletDynamicsCommon.h>


using namespace Hydra::System;

BulletSystem::BulletSystem() {}
BulletSystem::~BulletSystem() {}

void BulletSystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process WeaponComponent
	world::getEntitiesWithComponents<Hydra::Component::WeaponComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto w = entities[i]->getComponent<Hydra::Component::WeaponComponent>();

		if (w->fireRateTimer > 0)
			w->fireRateTimer -= delta;
	}

	//Process BulletComponent
	world::getEntitiesWithComponents<Hydra::Component::BulletComponent, Hydra::Component::TransformComponent, Hydra::Component::RigidBodyComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto b = entities[i]->getComponent<Hydra::Component::BulletComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		auto rbc = entities[i]->getComponent<Hydra::Component::RigidBodyComponent>();
		auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
		
		btVector3 temp = rigidBody->getLinearVelocity();
		if (temp.x() == 0)
		{
			rigidBody->applyCentralForce(btVector3(b->direction.x, b->direction.y, b->direction.z) * b->velocity);
		}

		// TESTING HOMNIG/MAGNETIC NOT REALLY GOOD RIGHT NOW
		/*if (b->bulletType == BULLETTYPE_HOMING) {
			float distance = 10000;
			int_openmp_t closestEnemy = -1;
			std::vector<std::shared_ptr<Hydra::World::Entity>> enemies;
			world::getEntitiesWithComponents<Hydra::Component::AIComponent, Hydra::Component::TransformComponent>(enemies);
			for (int_openmp_t i = 0; i < (int_openmp_t)enemies.size(); i++) {
				auto enemyT = enemies[i]->getComponent<Hydra::Component::TransformComponent>();
				float distanceToEnemy = glm::distance(t->position, enemyT->position);
				if (distanceToEnemy < distance){
					distance = distanceToEnemy;
					closestEnemy = i;
				}
			}
			if (closestEnemy != -1) {
				auto rbc = entities[i]->getComponent<Hydra::Component::RigidBodyComponent>();
				auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
				auto enemyT = enemies[closestEnemy]->getComponent<Hydra::Component::TransformComponent>();
				glm::vec3 enemyPos = enemyT->position;
				enemyPos.y += 2.0f;
				glm::vec3 direction = glm::normalize(enemyPos - t->position) * 100.f;
				rigidBody->applyCentralForce(btVector3(direction.x,direction.y,direction.z));
			}
		}*/

		b->deleteTimer -= delta;
		if (b->deleteTimer <= 0)
			entities[i]->dead = true;
	}

	entities.clear();
}

void BulletSystem::registerUI() {}
