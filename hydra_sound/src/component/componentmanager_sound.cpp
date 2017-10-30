#include <hydra/component/componentmanager_sound.hpp>
#include <hydra/component/soundfxcomponent.hpp>
using namespace Hydra::Component::ComponentManager;

using namespace Hydra::World;

namespace Hydra::Component::ComponentManager {
	void registerComponents_sound(std::map<std::string, createOrGetComponent_f>& creators) {
		(void)creators;
		SoundFxComponent::componentHandler = new ComponentHandler<SoundFxComponent>();
		creators["SoundFxComponent"] = &createOrGetComponentHelper<SoundFxComponent>;
	}
}
