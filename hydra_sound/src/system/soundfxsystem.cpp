#include <hydra/system/soundfxsystem.hpp>

#include <imgui/imgui.h>
#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>

#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/component/playercomponent.hpp>

#include <algorithm>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

SoundFxSystem::SoundFxSystem() {}
SoundFxSystem::~SoundFxSystem() {}

void SoundFxSystem::tick(float delta) {
	std::shared_ptr<Hydra::World::Entity> player;
	world::getEntitiesWithComponents<PlayerComponent>(entities);
	if (entities.empty())
		return;

	player = entities[0];
	auto playerCamera = player->getComponent<CameraComponent>();
	auto playerT = player->getComponent<TransformComponent>();
	glm::mat4 rotation = glm::mat4_cast(playerT->rotation);
	
	const glm::vec3 forward = glm::vec3(glm::vec4{ 0, 0, 1, 0 } * rotation);
	const glm::vec3 playerPos = player->getComponent<TransformComponent>()->position;

	world::getEntitiesWithComponents<SoundFxComponent, TransformComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();
		auto transform = entities[i]->getComponent<TransformComponent>();

		for (size_t i = 0; i < soundFx->soundsToPlay.size(); i++) {
			soundFx->playingSounds.push_back(Mix_LoadWAV(soundFx->soundsToPlay.back().c_str()));
			soundFx->playingChannels.push_back(Mix_PlayChannel(-1, soundFx->playingSounds.back(), 0));
			soundFx->soundsToPlay.pop_back();
		}

		for (size_t i = 0; i < soundFx->playingChannels.size(); i++){
			if (Mix_Playing(soundFx->playingChannels[i])){

				float range = glm::distance(transform->position, playerPos);
				Uint8 distance = std::min(255, (int)(range / 70 * 255));
				
				Sint16 angleToPlayer = 0;
				if (range != 0){
					float playerDirAngle = glm::degrees(atan2(forward.x, forward.z));
					float angle = glm::degrees(atan2(transform->position.x - playerPos.x, transform->position.z - playerPos.z));
					angleToPlayer = ((int)(angle - (int)playerDirAngle) + 360) % 360;
				}

				Mix_SetPosition(soundFx->playingChannels[i], angleToPlayer, distance);
			}
			else {
				Mix_FreeChunk(soundFx->playingSounds[i]);

				soundFx->playingChannels[i] = soundFx->playingChannels.back();
				soundFx->playingSounds[i] = soundFx->playingSounds.back();
				soundFx->playingChannels.pop_back();
				soundFx->playingSounds.pop_back();
			}
		}	
	}

	entities.clear();
}

void SoundFxSystem::registerUI() {}
