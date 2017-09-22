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
	class HYDRA_API LightComponent final : public IComponent{
	private:
		glm::vec3 _position;
		glm::quat _orientation;
		glm::vec3 _direction;

		float _fov = 90.0f;
		float _zNear = 0.001f;
		float _zFar = 200.0f;
		float _aspect = 1920.0f / 1080.0f;
	public:
		LightComponent(IEntity* entity);
		LightComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position = { 0, 0, 0 });
		~LightComponent() final;

		void setPosition(const glm::vec3& position);
		void setDirection(const glm::vec3& direction);
		inline const glm::vec3& getPosition() { return _position; }
		inline const glm::vec3& getDirection() { return _direction; }
		void translate(const glm::vec3& transform);

		//inline Hydra::Renderer::IRenderTarget* getRenderTarget() { return _renderTarget; }
		//inline void setRenderTarget(Hydra::Renderer::IRenderTarget* renderTarget) { _renderTarget = renderTarget; }

		//inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(_orientation), -_position); }
		//inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(_fov), _aspect, _zNear, _zFar); }
		inline glm::mat4 getProjectionMatrix() const { return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, _zNear, _zFar); }
		inline glm::mat4 getViewMatrix() const {
			return glm::lookAt(glm::vec3(_position.x, _position.y, _position.z),
				   glm::vec3(_position + _direction),
				   glm::vec3(0, 1, 0));
			//return glm::translate(glm::mat4_cast(_orientation), -_position);
		}

		void tick(TickAction action, float delta);
		TickAction wantTick() const { return TickAction::physics; }

		const std::string type() const final { return "LightComponent"; }

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;
	};
}