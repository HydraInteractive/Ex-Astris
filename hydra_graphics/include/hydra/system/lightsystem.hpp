#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class LightSystem final : public Hydra::World::ISystem {
	public:
		~LightSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "LightSystem"; }
		void registerUI() final;
	};
}
