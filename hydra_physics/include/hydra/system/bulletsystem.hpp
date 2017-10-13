#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API BulletSystem final : public Hydra::World::ISystem {
	public:
		BulletSystem();
		~BulletSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "BulletSystem"; }
		void registerUI() final;
	};
}
