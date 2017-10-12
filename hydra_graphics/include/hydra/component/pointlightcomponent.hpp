#pragma once

#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform2.hpp>

#include <hydra/component/transformcomponent.hpp>

#include <SDL2/SDL.h>

// Below are values to specific when the light should fall off. (Attenuation).
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
	class HYDRA_API PointLightComponent final : public IComponent{
	private:
		glm::vec3 _position;
		glm::vec3 _color;
		float _constant;
		float _linear;
		float _quadratic;
	public:
		PointLightComponent(IEntity* entity);
		PointLightComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position = { 0, 0, 0 });
		~PointLightComponent() final;

		void setPosition(const glm::vec3& position) { this->entity->getComponent<Hydra::Component::TransformComponent>()->setPosition(position); }
		void setColor(const glm::vec3& color) { _color = color; }
		inline const glm::vec3& getPosition() { return _position; }
		inline const glm::vec3& getColor() { return _color; }
		inline const float& getConstant() { return _constant; }
		inline const float& getLinear() { return _linear; }
		inline const float& getQuadratic() { return _quadratic; }

		void tick(TickAction action, float delta);
		TickAction wantTick() const { return TickAction::physics; }

		const std::string type() const final { return "PointLightComponent"; }

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;
	};
}