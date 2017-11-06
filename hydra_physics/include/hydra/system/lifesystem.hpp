#pragma once
#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API LifeSystem final : public Hydra::World::ISystem{
	public:
		LifeSystem();
		~LifeSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "LifeSystem"; }
		void registerUI() final;

	private:

	};
}