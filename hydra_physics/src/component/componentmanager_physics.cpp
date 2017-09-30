#include <hydra/component/componentmanager_physics.hpp>

using namespace Hydra::Component::ComponentManager;

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>

namespace Hydra::Component::ComponentManager {
	void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["PlayerComponent"] = &createOrGetComponentHelper<PlayerComponent>;
		creators["EnemyComponent"] = &createOrGetComponentHelper<EnemyComponent>;
		creators["RigidBodyComponent"] = &createOrGetComponentHelper<RigidBodyComponent>;
	}
}
