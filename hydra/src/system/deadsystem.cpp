#include <hydra/system/deadsystem.hpp>

using namespace Hydra::System;

DeadSystem::DeadSystem() {}
DeadSystem::~DeadSystem() {}

void DeadSystem::tick(float delta) {
	using world = Hydra::World::World;
	auto map = world::_map;
	for (auto& kv : map) {
		auto e = world::getEntity(kv.first);
		if (!e)
			continue;
		if (e->dead)
			world::removeEntity(e->id);
	}

	entities.clear();
}

void DeadSystem::registerUI() {}
