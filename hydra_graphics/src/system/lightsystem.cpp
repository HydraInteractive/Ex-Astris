#include <hydra/system/lightsystem.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>

using namespace Hydra::System;
using namespace Hydra::World;
using namespace Hydra::Component;

LightSystem::LightSystem() {
	
}

LightSystem::~LightSystem() {
	
}

void LightSystem::tick(float delta) {
	World::World::getEntitiesWithComponents<LightComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto lightTC = entities[i]->getComponent<TransformComponent>();
		auto lightParent = World::World::getEntity(entities[i]->parent);
		auto lightParentTC = lightParent->getComponent<TransformComponent>();
		
		if (lightParentTC && lightParentTC->dirty)
			lightTC->position = glm::vec3(0, 50, 0) + lightParentTC->position;
	}
}

void LightSystem::registerUI() {
	
}