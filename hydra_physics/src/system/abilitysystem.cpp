#include <hydra/system/abilitysystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <algorithm>

#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>

using namespace Hydra::System;

AbilitySystem::AbilitySystem() {}
AbilitySystem::~AbilitySystem() {}

void AbilitySystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process GrenadeComponent
	world::getEntitiesWithComponents<Hydra::Component::GrenadeComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto g = entities[i]->getComponent<Hydra::Component::GrenadeComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();

		//OLD CODE BEFORE BULLLETPHYSICS
		/*
		g->direction.y -= 0.8 * delta;
		g->velocity = std::max(0.0f, g->velocity - 2.0f * delta);

		if (g->fallingVelocity < 0)
			g->fallingVelocity = 0;

		glm::vec3 temp{g->velocity, g->fallingVelocity, g->velocity};

		t->position += g->direction * temp * delta;
		t->dirty = true;

		if (t->position.y < 0.0f) {
			t->position.y = 0.0f;
			g->fallingVelocity -= 10.0f;
			g->velocity -= 5.0f;
			g->direction.y = -g->direction.y;
		}

		if (g->velocity == 0 && g->fallingVelocity == 0)
			entities[i]->dead = true;
		*/
	}

	//Process MineComponent
	world::getEntitiesWithComponents<Hydra::Component::MineComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto m = entities[i]->getComponent<Hydra::Component::MineComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();

		//OLD CODE BEFORE BULLLETPHYSICS
		/*m->direction.y -= 0.8 * delta;
		m->velocity = std::max(0.0f, m->velocity - 2.0f * delta);

		if (m->fallingVelocity < 0)
			m->fallingVelocity = 0;

		glm::vec3 temp(m->velocity, m->fallingVelocity, m->velocity);

		t->position += m->direction * temp * delta;
		t->dirty = true;

		if (t->position.y < 0.0f) {
			t->position.y = 0.0f;
			m->velocity = 0.0f;
			m->fallingVelocity = 0.0f;
		}

		m->timer -= delta;
		if (m->timer <= 0)
		entities[i]->dead = true;*/
	}
}

void AbilitySystem::registerUI() {}
