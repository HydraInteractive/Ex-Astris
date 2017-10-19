#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API AISystem final : public Hydra::World::ISystem {
	public:
		AISystem();
		~AISystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "AISystem"; }
		void registerUI() final;
	};
}
