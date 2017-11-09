#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_BASE_API DeadSystem final : public Hydra::World::ISystem {
	public:
		DeadSystem();
		virtual ~DeadSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "DeadSystem"; }
		void registerUI() final;
	};
}
