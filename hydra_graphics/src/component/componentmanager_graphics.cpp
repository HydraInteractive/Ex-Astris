#include <hydra/component/componentmanager_graphics.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/EditorCameraComponent.hpp>

namespace Hydra::Component::ComponentManager {
	void registerComponents_graphics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["CameraComponent"] = &createOrGetComponentHelper<CameraComponent>;
		creators["MeshComponent"] = &createOrGetComponentHelper<MeshComponent>;
		creators["TransformComponent"] = &createOrGetComponentHelper<TransformComponent>;
		creators["LightComponent"] = &createOrGetComponentHelper<LightComponent>;
		creators["ParticleComponent"] = &createOrGetComponentHelper<ParticleComponent>;
		creators["EditorCameraComponent"] = &createOrGetComponentHelper<EditorCameraComponent>;
	}
}
