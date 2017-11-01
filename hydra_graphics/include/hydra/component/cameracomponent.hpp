/**
 * A camera component that generates view and projection matrices.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <hydra/component/transformcomponent.hpp>

#include <SDL2/SDL.h>

using namespace Hydra::World;

namespace Hydra::Component {

	struct HYDRA_GRAPHICS_API CameraComponent final : public IComponent<CameraComponent, ComponentBits::Camera> {
		glm::vec3 position = glm::vec3{0, 0, 0};
		glm::quat orientation = glm::quat();

		float fov = 90.0f;
		float zNear = 0.1f;
		float zFar = 75.0f;
		float aspect = 1920.0f/1080.0f;

		float sensitivity = 0.003f;
		float cameraYaw = 0.0f;
		float cameraPitch = 0.0f;
		bool mouseControl = true;

		//enum { TOP = 0, BOTTOM, LEFT, RIGHT, NEAR, FAR };

		~CameraComponent() final;

		inline const std::string type() const final { return "CameraComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(orientation), -position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(fov), aspect, zNear, zFar); }
	};
};
