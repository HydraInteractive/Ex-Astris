#include <hydra/component/componentmanager_physics.hpp>

using namespace Hydra::Component::ComponentManager;


#include <hydra/component/playercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>

namespace Hydra::Component::ComponentManager {
	void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["PlayerComponent"] = &createOrGetComponentHelper<PlayerComponent>;
		creators["EnemyComponent"] = &createOrGetComponentHelper<EnemyComponent>;
		creators["RigidBodyComponent"] = &createOrGetComponentHelper<RigidBodyComponent>;
		creators["WeaponComponent"] = &createOrGetComponentHelper<WeaponComponent>;
		creators["BulletComponent"] = &createOrGetComponentHelper<BulletComponent>;
		creators["GrenadeComponent"] = &createOrGetComponentHelper<GrenadeComponent>;
		creators["MineComponent"] = &createOrGetComponentHelper<MineComponent>;
	}
}
