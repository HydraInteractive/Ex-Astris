#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/component/componentmanager.hpp>

namespace Hydra::Component {
	namespace ComponentManager {
		HYDRA_API void registerComponents_network(std::map<std::string, createOrGetComponent_f>& creators);
	};
};
