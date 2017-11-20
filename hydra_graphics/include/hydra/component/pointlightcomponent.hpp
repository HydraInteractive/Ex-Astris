#pragma once

#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform2.hpp>

#include <hydra/component/transformcomponent.hpp>

#include <SDL2/SDL.h>

// Below are values to specify when the light should fall off. (Attenuation).
//Range constant linear quadratic
//3250, 1.0, 0.0014, 0.000007
//600, 1.0, 0.007, 0.0002
//325, 1.0, 0.014, 0.0007
//200, 1.0, 0.022, 0.0019
//160, 1.0, 0.027, 0.0028
//100, 1.0, 0.045, 0.0075
//65, 1.0, 0.07, 0.017
//50, 1.0, 0.09, 0.032
//32, 1.0, 0.14, 0.07
//20, 1.0, 0.22, 0.20
//13, 1.0, 0.35, 0.44
//7, 1.0, 0.7, 1.8


using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API PointLightComponent final : public IComponent<PointLightComponent, ComponentBits::PointLight> {
		glm::vec3 color = glm::vec3{1, 1, 1};
		float constant = 1;
		float linear = 0.045f;
		float quadratic = 0.0075f;

		~PointLightComponent() final;

		const std::string type() const final { return "PointLightComponent"; }

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;

		inline std::shared_ptr<Hydra::Component::TransformComponent> getTransformComponent() {
			if (auto e = Hydra::World::World::getEntity(entityID); e)
				return e->getComponent<Hydra::Component::TransformComponent>();
			else
				return std::shared_ptr<Hydra::Component::TransformComponent>();
		}
	};
}
