#include <hydra/system/soundfxsystem.hpp>

#include <imgui/imgui.h>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>

#include <hydra/engine.hpp>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

SoundFxSystem::SoundFxSystem() {}
SoundFxSystem::~SoundFxSystem() {}

void SoundFxSystem::tick(float delta) {
	world::getEntitiesWithComponents<SoundFxComponent, TransformComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();

		int channel = Mix_PlayChannel(-1, soundFx->testSound, 0);
	}
}

void SoundFxSystem::registerUI() {}