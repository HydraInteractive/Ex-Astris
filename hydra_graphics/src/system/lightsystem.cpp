#include <hydra/system/lightsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lightcomponent.hpp>

using namespace Hydra::System;

LightSystem::LightSystem() {}
LightSystem::~LightSystem() {}

void LightSystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process LightComponent
	world::getEntitiesWithComponents<Hydra::Component::LightComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto l = entities[i]->getComponent<Hydra::Component::LightComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		l->position = t->position;
	}
}

void LightSystem::registerUI() {}
