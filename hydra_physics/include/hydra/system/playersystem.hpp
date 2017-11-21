#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API PlayerSystem final : public Hydra::World::ISystem {
	private:
		float _dyaw, _dpitch = 0;
		bool _isReloading = false;
	public:
		PlayerSystem();
		~PlayerSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
