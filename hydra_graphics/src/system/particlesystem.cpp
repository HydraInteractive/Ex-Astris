#include <hydra/system/particlesystem.hpp>

#include <hydra/component/particlecomponent.hpp>

using namespace Hydra::System;
ParticleSystem::~ParticleSystem() {}

void ParticleSystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process ParticleComponent
	world::getEntitiesWithComponents<Hydra::Component::ParticleComponent>(entities);
	#pragma omp parallel for
	for (size_t i = 0; i < entities.size(); i++) {
	}
}

void ParticleSystem::registerUI() {}
