#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API LightSystem final : public Hydra::World::ISystem {
	public:
		LightSystem();
		~LightSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "LightSystem"; }
		void registerUI() final;
	};
}
