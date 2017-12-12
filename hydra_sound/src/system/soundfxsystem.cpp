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
SoundFxSystem::~SoundFxSystem() {
	Mix_FreeMusic(music);
}

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
		auto transform = entities[i]->getComponent<TransformComponent>();
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();

		//Add sound effects
		for (size_t i = 0; i < soundFx->soundsToPlay.size(); i++) {

			//Try to reuse loaded soundfx
			std::vector<std::string>::iterator it = std::find(soundPath.begin(), soundPath.end(), soundFx->soundsToPlay.back());

			if (it != soundPath.end()){
				soundFx->playingChannels.push_back(Mix_PlayChannel(-1, soundChunk.at(it - soundPath.begin()), 0));
				Mix_ChannelFinished(removeChannelFromComponent);
			}else{
				auto chunk = Mix_LoadWAV(soundFx->soundsToPlay.back().c_str());
				soundChunk.push_back(chunk);
				soundPath.push_back(soundFx->soundsToPlay.back());
				soundFx->playingChannels.push_back(Mix_PlayChannel(-1, chunk, 0));
				Mix_ChannelFinished(removeChannelFromComponent);
			}

			soundFx->soundsToPlay.pop_back();
		}

		//Update sound effects position relative to the player
		for (size_t i = 0; i < soundFx->playingChannels.size(); i++){
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
	}
	entities.clear();
	
	//Removing used sound effects
	removeTimer -= delta;
	if (removeTimer <= 0 && soundChunk.size() > 0){
		bool* isInUse = new bool[soundChunk.size()]{};
		for (size_t i = 0; i < MIX_CHANNELS; i++){
			if (Mix_Playing(i)){
				for (size_t j = 0; j < soundChunk.size(); j++) {
					if (soundChunk[j] == Mix_GetChunk(i)) {
						isInUse[j] = true;
					}
				}
			}
		}
		for (size_t i = 0; i < soundChunk.size(); i++){
			if (!isInUse[i]){
				Mix_FreeChunk(soundChunk[i]);
				soundChunk[i] = soundChunk.back();
				soundPath[i] = soundPath.back();
				soundChunk.pop_back();
				soundPath.pop_back();
			}
		}
		delete[] isInUse;
		removeTimer = 5;
	}
	/*for (size_t j = 0; j < soundFx->soundsToPlay.size(); j++) {

		int found = -1;
		for (size_t k = 0; k < soundPath.size(); k++) {
			if (soundFx->soundsToPlay.back() == soundPath.back()) {
				found = k;
				k = soundPath.size();
			}
		}

		if (found != -1) {
			soundChunk.push_back(soundChunk[found]);
			soundPath.push_back(soundPath[found]);
			soundEntity.push_back(entities[i]->id);
			Mix_PlayChannel(-1, soundChunk.back(), 0);
			soundFx->soundsToPlay.pop_back();
		}
		else {
			soundChunk.push_back(Mix_LoadWAV(soundFx->soundsToPlay.back().c_str()));
			soundPath.push_back(soundFx->soundsToPlay.back());
			soundEntity.push_back(entities[i]->id);
			Mix_PlayChannel(-1, soundChunk.back(), 0);
			soundFx->soundsToPlay.pop_back();
		}
	}

	bool* isInUse = new bool[soundChunk.size()]{};
	for (size_t i = 0; i < MIX_CHANNELS; i++){
		if (Mix_Playing(i)){
			for (size_t j = 0; j < soundChunk.size(); j++){
				if (Mix_GetChunk(i) == soundChunk[j]){
					isInUse[j] = true;
					j = soundChunk.size();
				}
			}
		}
	}
	std::stringstream temp; temp << "SoundChunkSize = " << soundChunk.size() << std::endl;
	temp << "SoundPathSize = " << soundPath.size() << std::endl;
	temp << "SoundEntitySize = " << soundEntity.size() << std::endl;
	printf(temp.str().c_str());
	for (size_t i = 0; i < soundChunk.size(); i++){
		if (!isInUse[i]){
			soundPath[i] = soundPath.back();
			soundChunk[i] = soundChunk.back();
			soundEntity[i] = soundEntity.back();
			soundPath.pop_back();
			soundChunk.pop_back();
			soundEntity.pop_back();
			printf("SOUND HAS BEEN FREED\n");
		}
	}
	delete[] isInUse;*/
}

void Hydra::System::SoundFxSystem::removeChannelFromComponent(int channel){
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	world::getEntitiesWithComponents<SoundFxComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();
		
		auto it = std::find(soundFx->playingChannels.begin(), soundFx->playingChannels.end(), channel);
		if (it != soundFx->playingChannels.end()){
			int index = it - soundFx->playingChannels.begin();
			soundFx->playingChannels[i] = soundFx->playingChannels.back();
			soundFx->playingChannels.pop_back();
		}
	}
	entities.clear();
}

void Hydra::System::SoundFxSystem::startMusic(std::string songPath){
	music = Mix_LoadMUS(songPath.c_str());
	Mix_PlayMusic(music, -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME/4);
}

void SoundFxSystem::registerUI() {}
