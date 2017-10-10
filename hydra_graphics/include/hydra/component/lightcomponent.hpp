#pragma once

#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform2.hpp>

#include <hydra/component/transformcomponent.hpp>

#include <SDL2/SDL.h>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_API LightComponent final : public IComponent<LightComponent, ComponentBits::Light> {
		glm::vec3 position = glm::vec3{0, 0, 0};
		glm::vec3 direction = glm::vec3{0, 0, 0};

		float fov = 80.0f;
		float zNear = -10.f;
		float zFar = 20.0f;

		~LightComponent() final;

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;

		const std::string type() const final { return "LightComponent"; }

		inline glm::mat4 getProjectionMatrix() const { return glm::ortho(-10.f, 10.f, -10.f, 10.f, zNear, zFar); }
		//inline glm::mat4 getProjectionMatrix() const { return glm::perspective<float>(glm::radians(fov), 1.0f, zNear, zFar); }
		inline glm::mat4 getViewMatrix() const {
			return glm::lookAt(glm::vec3(position),
				   glm::vec3(position + direction),
				   glm::vec3(0, 1, 0));
			//return glm::translate(glm::mat4cast(orientation), -position);
		}
	};
}
