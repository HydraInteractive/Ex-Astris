#pragma once
#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API LifeSystem final : public Hydra::World::ISystem{
	public:
		LifeSystem();
		~LifeSystem() final;

		void tick(float delta) final;

		float ressurectTimer = 0.0f;

		inline const std::string type() const final { return "LifeSystem"; }
		void registerUI() final;

	private:

	};
}