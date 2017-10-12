#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class PlayerSystem final : public Hydra::World::ISystem {
	public:
		~PlayerSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
