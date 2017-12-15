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
	world::getEntitiesWithComponents<Hydra::Component::BulletComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto b = entities[i]->getComponent<Hydra::Component::BulletComponent>();

		b->deleteTimer -= delta;
		if (b->deleteTimer <= 0)
			entities[i]->dead = true;
	}

	entities.clear();
}

void BulletSystem::registerUI() {}
