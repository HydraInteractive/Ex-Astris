#include <hydra/system/aisystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/aicomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>

using namespace Hydra::System;

AISystem::AISystem() {}
AISystem::~AISystem() {}

void AISystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process AiComponent
	world::getEntitiesWithComponents<Component::AIComponent, Component::TransformComponent, Component::LifeComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto enemy = entities[i]->getComponent<Component::AIComponent>();
		enemy->behaviour->run(delta);
	}
}

void AISystem::registerUI() {}
