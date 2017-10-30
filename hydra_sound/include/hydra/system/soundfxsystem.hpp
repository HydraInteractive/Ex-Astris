#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_SOUND_API SoundFxSystem final : public Hydra::World::ISystem{
	public:
		SoundFxSystem();
		~SoundFxSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
