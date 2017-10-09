#include <hydra/system/deadsystem.hpp>

DeadSystem::~DeadSystem() {}

void DeadSystem::tick(float delta) {
	using world = Hydra::World::World;
	for (auto it = world::_entities.rbegin(); it != world::_entities.rend(); it++)
		if ((*it)->dead)
			world::removeEntity((*it)->id);
}

void DeadSystem::registerUI() {}
