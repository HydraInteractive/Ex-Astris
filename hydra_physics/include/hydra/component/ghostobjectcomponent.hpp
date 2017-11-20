#pragma once

#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API GhostObjectComponent final : public IComponent<GhostObjectComponent, ComponentBits::GhostObject>{
		btGhostObject* ghostObject = nullptr;
		Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType = Hydra::System::BulletPhysicsSystem::COLL_WALL;
		Hydra::System::BulletPhysicsSystem* _handler = nullptr;
		
		glm::vec3 halfExtents = glm::vec3(1.f,1.f,1.f);
		glm::vec3 rotation = glm::vec3(0.f,0.f,0.f);
		glm::quat quatRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
		
		glm::mat4 _matrix = glm::mat4(1);

		inline glm::mat4 getMatrix() {
			_recalculateMatrix();
			return _matrix;
		}

		void _recalculateMatrix();

		//void createSphere(float radius);
		void createBox(const glm::vec3& halfExtents, Hydra::System::BulletPhysicsSystem::CollisionTypes collType, const glm::quat& quatRotation = glm::quat(0,0,0,1));

		~GhostObjectComponent() final;

		inline const std::string type() const final { return "GhostObjectComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
