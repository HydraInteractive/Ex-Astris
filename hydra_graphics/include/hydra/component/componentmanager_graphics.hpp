#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/component/componentmanager.hpp>

namespace Hydra::Component {
	namespace ComponentManager {
		HYDRA_GRAPHICS_API void registerComponents_graphics(std::map<std::string, createOrGetComponent_f>& creators);
	};
};