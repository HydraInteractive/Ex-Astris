#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API PickUpSystem final : public Hydra::World::ISystem{
	public:
		PickUpSystem();
		~PickUpSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
