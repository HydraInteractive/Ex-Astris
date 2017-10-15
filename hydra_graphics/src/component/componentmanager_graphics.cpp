#include <hydra/component/componentmanager_graphics.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/EditorCameraComponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component::ComponentManager {
	void registerComponents_graphics(std::map<std::string, createOrGetComponent_f>& creators) {
		CameraComponent::componentHandler = new ComponentHandler<CameraComponent>();
		creators["CameraComponent"] = &createOrGetComponentHelper<CameraComponent>;
		MeshComponent::componentHandler = new ComponentHandler<MeshComponent>();
		creators["MeshComponent"] = &createOrGetComponentHelper<MeshComponent>;
		LightComponent::componentHandler = new ComponentHandler<LightComponent>();
		creators["LightComponent"] = &createOrGetComponentHelper<LightComponent>;
		ParticleComponent::componentHandler = new ComponentHandler<ParticleComponent>();
		creators["ParticleComponent"] = &createOrGetComponentHelper<ParticleComponent>;
		EditorCameraComponent::componentHandler = new ComponentHandler<EditorCameraComponent>();
		creators["EditorCameraComponent"] = &createOrGetComponentHelper<EditorCameraComponent>;
		DrawObjectComponent::componentHandler = new ComponentHandler<DrawObjectComponent>();
		creators["DrawObjectComponent"] = &createOrGetComponentHelper<DrawObjectComponent>;
	}
}
