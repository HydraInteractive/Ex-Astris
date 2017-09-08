#include <hydra/component/componentmanager.hpp>

namespace Hydra::Component::ComponentManager {
	std::map<std::string, createOrGetComponent_f>& createOrGetComponentMap() {
		static std::map<std::string, createOrGetComponent_f> map;
		return map;
	}
}
