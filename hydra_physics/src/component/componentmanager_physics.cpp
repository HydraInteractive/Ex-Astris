#include <hydra/component/componentmanager_physics.hpp>

using namespace Hydra::Component::ComponentManager;

namespace Hydra::Component::ComponentManager {
	void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["PlayerComponent"] = &createOrGetComponentHelper<PlayerComponent>;
		creators["EnemyComponent"] = &createOrGetComponentHelper<EnemyComponent>;
		creators["WeaponComponent"] = &createOrGetComponentHelper<WeaponComponent>;
		creators["BulletComponent"] = &createOrGetComponentHelper<BulletComponent>;
	}
}
