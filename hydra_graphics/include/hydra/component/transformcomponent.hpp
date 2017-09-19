/**
 * A transform component that specifies that the entity has an position inside the world.
 * It contains a DrawObject instance from the renderer, to be able to update its location.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API TransformComponent final : public IComponent {
	public:
		TransformComponent(IEntity* entity, const glm::vec3& position = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::quat& rotation = glm::quat());
		~TransformComponent() final;

		void tick(TickAction action, float delta) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "TransformComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

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
