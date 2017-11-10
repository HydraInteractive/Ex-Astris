#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/component/transformcomponent.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API FreeCameraComponent final : public IComponent<FreeCameraComponent, ComponentBits::EditorCamera> {
		float movementSpeed = 10.0f;
		float shiftMultiplier = 5.0f;

		float fov = 90.0f;
		float zNear = 0.001f;
		float zFar = 75.0f;
		float aspect = 16.0f/9.0f;

		float sensitivity = 0.003f;
		float cameraYaw = 0.0f;
		float cameraPitch = 0.0f;

		~FreeCameraComponent() final;

		inline const std::string type() const final { return "FreeCameraComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() { return glm::translate(glm::mat4_cast(getTransformComponent()->rotation), -getTransformComponent()->position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(fov), aspect, zNear, zFar); }
		inline std::shared_ptr<Hydra::Component::TransformComponent> getTransformComponent() { return Hydra::World::World::getEntity(entityID)->getComponent<Hydra::Component::TransformComponent>(); }
	};
};

