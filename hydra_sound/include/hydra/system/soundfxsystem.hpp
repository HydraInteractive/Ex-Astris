#pragma once

#include <hydra/world/world.hpp>
#include <SDL2/SDL_mixer.h>

namespace Hydra::System {
	class HYDRA_SOUND_API SoundFxSystem final : public Hydra::World::ISystem{
	public:
		SoundFxSystem();
		~SoundFxSystem() final;

		void tick(float delta) final;
		inline const std::string type() const final { return "SoundFxSystem"; }
		void registerUI() final;
		
		static void removeChannelFromComponent(int channel);
	private:
		std::vector<std::string> soundPath = std::vector<std::string>();
		std::vector<Mix_Chunk*> soundChunk = std::vector<Mix_Chunk*>();
		float removeTimer = 5;
	};
}
