#pragma once
#include <hydra/ext/api.hpp>

#include <map>

#include <hydra/world/world.hpp>

namespace Hydra::Component {
	using namespace Hydra::World;
	namespace ComponentManager {
		typedef std::shared_ptr<IComponentBase> (*createOrGetComponent_f)(Entity* entity);

		template <typename T, typename std::enable_if<std::is_base_of<IComponentBase, T>::value>::type* = nullptr>
		static std::shared_ptr<IComponentBase> createOrGetComponentHelper(Entity* entity) {
			return entity->addComponent<T>();
		}

		HYDRA_API std::map<std::string /* Component name */, createOrGetComponent_f>& createOrGetComponentMap();
	};
};
