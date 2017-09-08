#include <hydra/component/componentmanager_network.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Component::ComponentManager {
	void registerComponents_graphics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["CameraComponent"] = &createOrGetComponentHelper<CameraComponent>;
		creators["MeshComponent"] = &createOrGetComponentHelper<MeshComponent>;
		creators["TranformComponent"] = &createOrGetComponentHelper<TransformComponent>;
	}
}
