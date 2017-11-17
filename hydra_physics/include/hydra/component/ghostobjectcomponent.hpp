#pragma once

#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API GhostObjectComponent final : public IComponent<GhostObjectComponent, ComponentBits::GhostObject>{
		
		
		btGhostObject* ghostObject;
		glm::vec3 halfExtents = glm::vec3();

		//void createSphere(float radius);
		void createBox(const glm::vec3& halfExtents);

		~GhostObjectComponent() final;

		inline const std::string type() const final { return "GhostObjectComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
