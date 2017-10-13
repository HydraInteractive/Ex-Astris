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
	struct HYDRA_BASE_API TransformComponent final : public IComponent<TransformComponent, ComponentBits::Transform> {
		glm::vec3 position = glm::vec3{0, 0, 0};
		glm::vec3 scale = glm::vec3{1, 1, 1};
		glm::quat rotation = glm::quat();

		bool ignoreParent = false;
		bool dirty = true; // Set this to true whenever some is changed

		//TransformComponent(IEntity* entity, const glm::vec3& position = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::quat& rotation = glm::quat());
		~TransformComponent() final;

		inline const std::string type() const final { return "TransformComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		inline glm::mat4 getMatrix() {
			auto p = _getParentComponent();
			if (dirty || (p && p->dirty))
				_recalculateMatrix();
			return _matrix;
		}

		inline glm::vec3 getDirection() {
			static const glm::vec3 forward = {0, 0, -1};
			return glm::mat3_cast(rotation) * forward;
		}

		inline void setPosition(const glm::vec3& position) {
			dirty = true;
			this->position = position;
		}
		inline void setScale(const glm::vec3& scale) {
			dirty = true;
			this->scale = scale;
		}
		inline void setRotation(const glm::quat& rotation) {
			dirty = true;
			this->rotation = rotation;
		}
		inline void setDirection(const glm::vec3& direction, glm::vec3 up = glm::vec3{0, 1, 0}) {
			if (direction == up)
				up.x += 0.0001f;

			static const glm::vec3 O = {0, 0, 0};
			glm::mat3 m = glm::lookAt(O, direction, up);
			setRotation(glm::quat_cast(m));
		}

	private:
		glm::mat4 _matrix;

		void _recalculateMatrix();
		std::shared_ptr<Hydra::Component::TransformComponent> _getParentComponent();
	};
	template struct HYDRA_BASE_API Hydra::World::IComponent<Hydra::Component::TransformComponent, Hydra::Component::ComponentBits::Transform>;
};
