#pragma once

#include <hydra/world/world.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/playercomponent.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API PerkSystem final : public Hydra::World::ISystem{
	public:
		PerkSystem();
		~PerkSystem() final;

		void tick(float delta) final;
		void onActivation(Perk newPerk, Entity* entity);
		void onTick(Perk activePerk);

		inline const std::string type() const final { return "PlayerSystem"; }
		void registerUI() final;
	};
}
