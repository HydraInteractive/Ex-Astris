#pragma once
#include <hydra/ext/api.hpp>

#include <map>

#include <hydra/world/world.hpp>

namespace Hydra::Component {
	using namespace Hydra::World;
	namespace ComponentManager {
		typedef IComponent* (*createOrGetComponent_f)(IEntity* entity);

		template <typename T, typename std::enable_if<std::is_base_of<IComponent, T>::value>::type* = nullptr>
		static IComponent* createOrGetComponentHelper(IEntity* entity) {
			T* c = entity->getComponent<T>();
			if (!c)
				entity->addComponent<T>();
			return c;
		}

		HYDRA_API std::map<std::string /* Component name */, createOrGetComponent_f>& createOrGetComponentMap();
	};
};
