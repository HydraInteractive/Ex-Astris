#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/component/componentmanager.hpp>

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>

namespace Hydra::Component {
	namespace ComponentManager {
		HYDRA_API void registerComponents_physics(std::map<std::string, createOrGetComponent_f>& creators);
	};
};
