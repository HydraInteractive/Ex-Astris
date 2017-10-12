#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class CameraSystem final : public Hydra::World::ISystem {
	public:
		~CameraSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "CameraSystem"; }
		void registerUI() final;
	};
}
