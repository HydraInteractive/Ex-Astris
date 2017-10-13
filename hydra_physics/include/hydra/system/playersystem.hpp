#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API PlayerSystem final : public Hydra::World::ISystem {
	public:
		PlayerSystem();
		~PlayerSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
