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
		void onPickUp(Hydra::Component::PerkComponent::Perk newPerk, const std::shared_ptr<Hydra::World::Entity>& playerEntity);

		inline const std::string type() const final { return "PerkSystem"; }
		void registerUI() final;
	};
}
