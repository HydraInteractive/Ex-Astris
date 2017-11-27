#include <hydra/component/componentmanager_physics.hpp>

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/spawnpointcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component::ComponentManager {
	void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators) {
		PlayerComponent::componentHandler = new ComponentHandler<PlayerComponent>();
		creators["PlayerComponent"] = &createOrGetComponentHelper<PlayerComponent>;
		AIComponent::componentHandler = new ComponentHandler<AIComponent>();
		creators["AIComponent"] = &createOrGetComponentHelper<AIComponent>;
		RigidBodyComponent::componentHandler = new ComponentHandler<RigidBodyComponent>();
		creators["RigidBodyComponent"] = &createOrGetComponentHelper<RigidBodyComponent>;
		WeaponComponent::componentHandler = new ComponentHandler<WeaponComponent>();
		creators["WeaponComponent"] = &createOrGetComponentHelper<WeaponComponent>;
		BulletComponent::componentHandler = new ComponentHandler<BulletComponent>();
		creators["BulletComponent"] = &createOrGetComponentHelper<BulletComponent>;
		GrenadeComponent::componentHandler = new ComponentHandler<GrenadeComponent>();
		creators["GrenadeComponent"] = &createOrGetComponentHelper<GrenadeComponent>;
		MineComponent::componentHandler = new ComponentHandler<MineComponent>();
		creators["MineComponent"] = &createOrGetComponentHelper<MineComponent>;
		LifeComponent::componentHandler = new ComponentHandler<LifeComponent>();
		creators["LifeComponent"] = &createOrGetComponentHelper<LifeComponent>;
		MovementComponent::componentHandler = new ComponentHandler<MovementComponent>();
		creators["MovementComponent"] = &createOrGetComponentHelper<MovementComponent>;
		SpawnerComponent::componentHandler = new ComponentHandler<SpawnerComponent>();
		creators["SpawnerComponent"] = &createOrGetComponentHelper<SpawnerComponent>;
		PerkComponent::componentHandler = new ComponentHandler<PerkComponent>();
		creators["PerkComponent"] = &createOrGetComponentHelper<PerkComponent>;
		PickUpComponent::componentHandler = new ComponentHandler<PickUpComponent>();
		creators["PickUpComponent"] = &createOrGetComponentHelper<PickUpComponent>;
		GhostObjectComponent::componentHandler = new ComponentHandler<GhostObjectComponent>();
		creators["GhostObjectComponent"] = &createOrGetComponentHelper<GhostObjectComponent>;
		SpawnPointComponent::componentHandler = new ComponentHandler<SpawnPointComponent>();
		creators["SpawnPointComponent"] = &createOrGetComponentHelper<SpawnPointComponent>;
	}
}
