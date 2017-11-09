#include <hydra/component/componentmanager.hpp>

#include <hydra/world/world.hpp>
#include <algorithm>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/roomcomponent.hpp>

namespace Hydra::Component::ComponentManager {
	std::map<std::string, createOrGetComponent_f>& createOrGetComponentMap() {
		static std::map<std::string, createOrGetComponent_f> map;
		if (map.empty()) {
			IComponent<Hydra::Component::TransformComponent, Hydra::Component::ComponentBits::Transform>::componentHandler = new ComponentHandler<TransformComponent>();
			map["TransformComponent"] = &createOrGetComponentHelper<TransformComponent>;
			IComponent<Hydra::Component::RoomComponent, Hydra::Component::ComponentBits::Room>::componentHandler = new ComponentHandler<RoomComponent>();
			map["RoomComponent"] = &createOrGetComponentHelper<RoomComponent>;
		}
		return map;
	}
}
