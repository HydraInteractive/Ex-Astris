#include <hydra/component/componentmanager_network.hpp>

using namespace Hydra::World;

#include <hydra/component/networksynccomponent.hpp>

namespace Hydra::Component::ComponentManager {
	void registerComponents_network(std::map<std::string, createOrGetComponent_f>& creators) {
		NetworkSyncComponent::componentHandler = new ComponentHandler<NetworkSyncComponent>();
		creators["NetworkSyncComponent"] = &createOrGetComponentHelper<NetworkSyncComponent>;
	}
}
