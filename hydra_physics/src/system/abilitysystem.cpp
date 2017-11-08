#include <hydra/system/abilitysystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <algorithm>

#include <hydra/component/soundfxcomponent.hpp>
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
		auto s = entities[i]->getComponent<Hydra::Component::SoundFxComponent>();
		g->detonateTimer -= delta;

		if (g->detonateTimer <= 0 && !g->isExploding) {
			s->soundsToPlay.push_back("assets/sounds/piano.wav");
			g->isExploding = true;
		}
	}

	//Process MineComponent
	world::getEntitiesWithComponents<Hydra::Component::MineComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto m = entities[i]->getComponent<Hydra::Component::MineComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
	}
}

void AbilitySystem::registerUI() {}
