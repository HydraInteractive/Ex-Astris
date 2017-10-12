#include <hydra/component/componentmanager_graphics.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/EditorCameraComponent.hpp>

using namespace Hydra::Component;
typedef Hydra::World::IComponent<CameraComponent, ComponentBits::Camera> camera;
typedef Hydra::World::IComponent<LightComponent, ComponentBits::Light> light;
typedef Hydra::World::IComponent<MeshComponent, ComponentBits::Mesh> mesh;
typedef Hydra::World::IComponent<ParticleComponent, ComponentBits::Particle> particle;
typedef Hydra::World::IComponent<EditorCameraComponent, ComponentBits::EditorCamera> editorCamera;
typedef Hydra::World::IComponent<DrawObjectComponent, ComponentBits::DrawObject> drawObject;

namespace Hydra::Component::ComponentManager {
	void registerComponents_graphics(std::map<std::string, createOrGetComponent_f>& creators) {
		creators["CameraComponent"] = &createOrGetComponentHelper<CameraComponent>;
		creators["MeshComponent"] = &createOrGetComponentHelper<MeshComponent>;
		creators["LightComponent"] = &createOrGetComponentHelper<LightComponent>;
		creators["ParticleComponent"] = &createOrGetComponentHelper<ParticleComponent>;
		creators["EditorCameraComponent"] = &createOrGetComponentHelper<EditorCameraComponent>;
		creators["DrawObjectComponent"] = &createOrGetComponentHelper<DrawObjectComponent>;
	}
}
