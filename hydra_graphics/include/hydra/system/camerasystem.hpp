#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API CameraSystem final : public Hydra::World::ISystem {
	public:
		CameraSystem();
		~CameraSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "CameraSystem"; }
		void registerUI() final;
	};
}
