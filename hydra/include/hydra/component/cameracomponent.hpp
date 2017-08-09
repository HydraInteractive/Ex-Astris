#pragma once

#include <hydra/world/world.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class CameraComponent final : public IComponent {
	public:
		CameraComponent(IEntity* entity, const glm::vec3& position = {0, 0, 0});
		~CameraComponent() final;

		void tick(TickAction action) final;

		inline const std::string type() const final { return "CameraComponent"; }

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const final;
		void registerUI() final;

		void translate(const glm::vec3& transform);
		void rotation(float angle, const glm::vec3& axis);

		CameraComponent& yaw(float angle);
		CameraComponent& pitch(float angle);
		CameraComponent& roll(float angle);

		inline const glm::vec3& getPosition() const { return _position; }

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(_orientation), _position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(_fov), _aspect, _zNear, _zFar); }

	private:
		glm::vec3 _position;
		glm::quat _orientation;

		float _fov = 90;
		float _zNear = 0.001;
		float _zFar = 75;
		float _aspect = 1920.0/1080.0;
	};
};
