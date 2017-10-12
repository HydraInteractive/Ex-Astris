#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class BulletSystem final : public Hydra::World::ISystem {
	public:
		~BulletSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "BulletSystem"; }
		void registerUI() final;
	};
}
