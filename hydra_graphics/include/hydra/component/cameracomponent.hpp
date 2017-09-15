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
	class HYDRA_API CameraComponent final : public IComponent{
	public:
		CameraComponent(IEntity* entity);
		CameraComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position = {0, 0, 0});
		~CameraComponent() final;

		void tick(TickAction action) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "CameraComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		void translate(const glm::vec3& transform);
		void rotation(float angle, const glm::vec3& axis);

		CameraComponent& yaw(float angle);
		CameraComponent& pitch(float angle);
		CameraComponent& roll(float angle);

		inline Hydra::Renderer::IRenderTarget* getRenderTarget() { return _renderTarget; }
		inline void setRenderTarget(Hydra::Renderer::IRenderTarget* renderTarget) { _renderTarget = renderTarget; }

		inline const glm::vec3& getPosition() const { return _position; }

		void setPosition(const glm::vec3& position);

		inline const float& getYaw() const { return _cameraYaw; }
		inline const float& getPitch() const { return _cameraPitch; }

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(_orientation), -_position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(_fov), (_renderTarget->getSize().x*1.0f) / _renderTarget->getSize().y, _zNear, _zFar); }

	private:
		Hydra::Renderer::IRenderTarget* _renderTarget;
		glm::vec3 _position;
		glm::quat _orientation;

		float _fov = 90.0f;
		float _zNear = 0.001f;
		float _zFar = 75.0f;
		float _aspect = 1920.0f/1080.0f;

		float _sensitivity = 0.003f;
		float _cameraYaw = 0.0f;
		float _cameraPitch = 0.0f;
		bool _mouseControl = true;
	};
};
