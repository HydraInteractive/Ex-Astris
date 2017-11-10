#include <hydra/system/lightsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>

using namespace Hydra::System;

LightSystem::LightSystem() {}
LightSystem::~LightSystem() {}

void LightSystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process LightComponent - Updates that used to be done here are now in the transform component exclusively
	//world::getEntitiesWithComponents<Hydra::Component::LightComponent, Hydra::Component::TransformComponent>(entities);
	//#pragma omp parallel for
	//for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
	//	auto l = entities[i]->getComponent<Hydra::Component::LightComponent>();
	//	auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
	//}

	//Process PointLightComponent - Updates that used to be done here are now in the transform component exclusively
	//world::getEntitiesWithComponents<Hydra::Component::PointLightComponent, Hydra::Component::TransformComponent>(entities);
	//#pragma omp parallel for
	//for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
	//	auto l = entities[i]->getComponent<Hydra::Component::PointLightComponent>();
	//	auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
	//}
}

void LightSystem::registerUI() {}
