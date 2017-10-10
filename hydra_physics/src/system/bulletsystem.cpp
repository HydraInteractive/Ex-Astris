#include <hydra/system/bulletsystem.hpp>

#include <hydra/component/bulletcomponent.hpp>

BulletSystem::~BulletSystem() {}

void BulletSystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process BulletComponent
	world::getEntitiesWithComponents<Hydra::Component::BulletComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (size_t i = 0; i < entities.size(); i++) {
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
