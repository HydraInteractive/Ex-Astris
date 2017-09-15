#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/component/componentmanager.hpp>

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/aicomponent.hpp>

namespace Hydra::Component {
	namespace ComponentManager {
		HYDRA_API void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators);
	};
};
