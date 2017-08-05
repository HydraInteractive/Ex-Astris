#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class TransformComponent final : public IComponent {
	public:
		TransformComponent(IEntity* entity, const glm::vec3& position = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::quat& rotation = glm::quat());
		~TransformComponent() final;

		void tick(TickAction action) final;

		inline const std::string type() final { return "TransformComponent"; }

		msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const final;

		inline bool isDirty() { return _dirty; }
		inline glm::mat4 getMatrix() {
			auto p = _getParentComponent();
			if (_dirty || (p && p->isDirty()))
				_recalculateMatrix();
			return _matrix;
		}
		inline glm::vec3 getPosition() { return _position; }
		inline glm::vec3 getScale() { return _scale; }
		inline glm::quat getRotation() { return _rotation; }
		inline glm::vec3 getDirection() {
			static const glm::vec3 forward = {0, 0, -1};
			return glm::mat3_cast(_rotation) * forward;
		}


		void setPosition(const glm::vec3& position);
		void setScale(const glm::vec3& scale);
		void setRotation(const glm::quat& rotation);
		void setDirection(const glm::vec3& direction, glm::vec3 up = glm::vec3{0, 1, 0});

	private:
		Hydra::Renderer::DrawObject* _drawObject;
		bool _dirty;

		glm::vec3 _position;
		glm::vec3 _scale;
		glm::quat _rotation;

		glm::mat4 _matrix;

		void _recalculateMatrix();
		TransformComponent* _getParentComponent();
	};
};
