#include <hydra/system/aisystem.hpp>

#include <hydra/component/aicomponent.hpp>

AISystem::~AISystem() {}

void AISystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process AiComponent
	world::getEntitiesWithComponents<Hydra::Component::EnemyComponent>(entities);
	#pragma omp parallel for
	for (size_t i = 0; i < entities.size(); i++) {
		auto e = entities[i]->getComponent<Hydra::Component::EnemyComponent>();

		//TODO: FIX!!!
		e->tick(delta);
	}
}

void AISystem::registerUI() {}
