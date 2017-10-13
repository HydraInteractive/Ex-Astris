#include <hydra/system/aisystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/aicomponent.hpp>

using namespace Hydra::System;

AISystem::AISystem() {}
AISystem::~AISystem() {}

void AISystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process AiComponent
	world::getEntitiesWithComponents<Hydra::Component::EnemyComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto e = entities[i]->getComponent<Hydra::Component::EnemyComponent>();

		//TODO: FIX!!!
		e->tick(delta);
	}
}

void AISystem::registerUI() {}
