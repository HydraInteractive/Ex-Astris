#include <hydra/system/lifesystem.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/particlecomponent.hpp>

#include <hydra/ext/openmp.hpp>

using namespace Hydra::System;

LifeSystem::LifeSystem() {
	
}

LifeSystem::~LifeSystem() {
	
}

void LifeSystem::tick(float delta) {
	using world = Hydra::World::World;

	world::getEntitiesWithComponents<Hydra::Component::LifeComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto lifeC = entities[i]->getComponent<Hydra::Component::LifeComponent>();
		if (lifeC->health <= 0)
			entities[i]->dead = true;

		if (entities[i]->getComponent<Hydra::Component::ParticleComponent>())
			lifeC->health -= 1 * delta;
	}

	entities.clear();
}

void LifeSystem::registerUI() {
	
}
