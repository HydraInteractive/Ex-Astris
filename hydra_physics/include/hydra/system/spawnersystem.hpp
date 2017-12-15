#pragma once

#include <hydra/world/world.hpp>
#include <hydra/component/weaponcomponent.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API SpawnerSystem final : public Hydra::World::ISystem{
	public:
		std::vector<Hydra::World::Entity*> didJustSpawn;	
		Hydra::Component::WeaponComponent::onShoot_f _onRobotShoot;
		void* _userdata;

		SpawnerSystem();
		~SpawnerSystem() final;

		void tick(float delta) final;
		inline const std::string type() const final { return "SpawnerSystem"; }
		void registerUI() final;
	};

}