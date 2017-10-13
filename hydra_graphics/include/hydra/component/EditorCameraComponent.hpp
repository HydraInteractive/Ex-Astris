#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API EditorCameraComponent final : public IComponent<EditorCameraComponent, ComponentBits::EditorCamera> {
		Hydra::Renderer::IRenderTarget* renderTarget = nullptr;

		glm::vec3 position = glm::vec3(0, 5, 0);
		float movementSpeed = 10.0f;
		float shiftMultiplier = 5.0f;

		glm::quat orientation = glm::quat();

		float fov = 90.0f;
		float zNear = 0.001f;
		float zFar = 75.0f;
		float aspect = 16.0f/9.0f;

		float sensitivity = 0.003f;
		float cameraYaw = 0.0f;
		float cameraPitch = 0.0f;

		~EditorCameraComponent() final;

		inline const std::string type() const final { return "EditorCameraComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(orientation), -position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(fov), (renderTarget->getSize().x*1.0f) / renderTarget->getSize().y, zNear, zFar); }
	};
	template struct HYDRA_GRAPHICS_API Hydra::World::IComponent<Hydra::Component::EditorCameraComponent, Hydra::Component::ComponentBits::EditorCamera>;
};

