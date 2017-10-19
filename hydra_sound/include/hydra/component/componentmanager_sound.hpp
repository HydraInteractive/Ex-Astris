#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/component/componentmanager.hpp>

namespace Hydra::Component {
	namespace ComponentManager {
		HYDRA_SOUND_API void registerComponents_sound(std::map<std::string, createOrGetComponent_f>& creators);
	};
};
