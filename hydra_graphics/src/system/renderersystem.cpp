#include <hydra/system/renderersystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>

using namespace Hydra::System;

RendererSystem::RendererSystem() {}
RendererSystem::~RendererSystem() {}

void RendererSystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process ParticleComponent
	world::getEntitiesWithComponents<Hydra::Component::DrawObjectComponent, Hydra::Component::TransformComponent>(entities);
#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto d = entities[i]->getComponent<Hydra::Component::DrawObjectComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		d->drawObject->modelMatrix = t->getMatrix();
	}

	entities.clear();
}

void RendererSystem::registerUI() {}
