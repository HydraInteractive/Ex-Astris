#pragma once
#include <hydra/ext/api.hpp>

#include <map>

#include <hydra/world/world.hpp>

namespace Hydra::Component {
	using namespace Hydra::World;
	namespace ComponentManager {
		typedef IComponentBase* (*createOrGetComponent_f)(std::shared_ptr<Entity>& entity);

		template <typename T, typename std::enable_if<std::is_base_of<IComponentBase, T>::value>::type* = nullptr>
		static IComponentBase* createOrGetComponentHelper(std::shared_ptr<Entity>& entity) {
			return entity->addComponent<T>();
		}

		HYDRA_API std::map<std::string /* Component name */, createOrGetComponent_f>& createOrGetComponentMap();
	};
};
