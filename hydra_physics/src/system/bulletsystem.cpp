#include <hydra/system/bulletsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>

using namespace Hydra::System;

BulletSystem::~BulletSystem() {}

void BulletSystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process WeaponComponent
	world::getEntitiesWithComponents<Hydra::Component::WeaponComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto w = entities[i]->getComponent<Hydra::Component::WeaponComponent>();

		if (w->fireRateTimer > 0)
			w->fireRateTimer -= delta;
	}

	//Process BulletComponent
	world::getEntitiesWithComponents<Hydra::Component::BulletComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto b = entities[i]->getComponent<Hydra::Component::BulletComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();

		b->position += b->velocity * b->direction * delta;
		t->position = b->position;

		b->deleteTimer -= delta;
		if (b->deleteTimer <= 0)
			entities[i]->dead = true;
	}
}

void BulletSystem::registerUI() {}
