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
	struct HYDRA_GRAPHICS_API LightComponent final : public IComponent<LightComponent, ComponentBits::Light> {
		glm::vec3 color = glm::vec3(1, 1, 1);

		float fov = 80.0f;
		float zNear = -20.0f;
		float zFar = 40.0f;
		float xNear = -14.0f;
		float xFar = 14.0f;
		float yNear = -14.0f;
		float yFar = 14.0f;
		~LightComponent() final;

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;

		const std::string type() const final { return "LightComponent"; }

		inline glm::mat4 getProjectionMatrix() const { return glm::ortho(xNear, xFar, yNear, yFar, zNear, zFar); }
		//inline glm::mat4 getProjectionMatrix() const { return glm::perspective<float>(fov, 1.0f, zNear, zFar); }
		inline glm::mat4 getViewMatrix() { return glm::translate(glm::mat4_cast(getTransformComponent()->rotation), -getTransformComponent()->position); }
		inline std::shared_ptr<Hydra::Component::TransformComponent> getTransformComponent() {
			if (auto e = Hydra::World::World::getEntity(entityID); e)
				return e->getComponent<Hydra::Component::TransformComponent>();
			else
				return std::shared_ptr<Hydra::Component::TransformComponent>();
		}
		inline glm::vec3 getDirVec(){
			return glm::normalize(getTransformComponent()->rotation * glm::vec3(0, 0, -1));
		}
	};
}
