#include <hydra/system/lightsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>

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
		// ALWAYS ORIENTAT AROUND FIXED UP AND FORWARD.
		l->direction = t->rotation * glm::vec3(0,0,-1);
		l->up = t->rotation * glm::vec3(0,1,0);
	}

	//Process PointLightComponent
	world::getEntitiesWithComponents<Hydra::Component::PointLightComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto l = entities[i]->getComponent<Hydra::Component::PointLightComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		l->position = t->position;
	}
}

void LightSystem::registerUI() {}
