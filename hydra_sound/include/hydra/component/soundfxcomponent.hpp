/**
* Sound Fx
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_mixer.h>
#include <hydra/world/world.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_SOUND_API SoundFxComponent final : public IComponent<SoundFxComponent, ComponentBits::SoundFx>{
	
	std::vector<std::string> soundsToPlay;
	std::vector<int> playingChannels;
	std::vector<Mix_Chunk*> playingSounds;

	~SoundFxComponent() final;

	inline const std::string type() const final { return "SoundFxComponent"; }
	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};