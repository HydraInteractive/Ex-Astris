#include <hydra/system/animationsystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>

using namespace Hydra::System;

AnimationSystem::AnimationSystem() {}
AnimationSystem::~AnimationSystem() {}

void AnimationSystem::tick(float delta) {
	World::World::getEntitiesWithComponents<Hydra::Component::MeshComponent, Hydra::Component::DrawObjectComponent>(entities);

	for(int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto drawObj = entities[i]->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
		auto mesh = drawObj->mesh;
		if (mesh->hasAnimation() == false || drawObj->disable || !mesh)
			continue;

		auto& mc = entities[i]->getComponent<Hydra::Component::MeshComponent>();
		int currentFrame = mc->currentFrame;
		float animationCounter = mc->animationCounter;

		if (animationCounter > 1 / 24.0f && currentFrame < mc->mesh->getMaxFramesForAnimation(mc->animationIndex)) {
			mc->animationCounter -= 1 / 24.0f;
			mc->currentFrame += 1;
		}
		else if (currentFrame >= mc->mesh->getMaxFramesForAnimation(mc->animationIndex))
			mc->currentFrame = 1;

		mc->animationCounter += 1 * delta;
	}
	entities.clear();
}


void AnimationSystem::registerUI() {}
