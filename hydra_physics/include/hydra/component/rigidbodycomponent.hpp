
/**
 * A component that enables the entity to be updated with physics.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/system/bulletphysicssystem.hpp>

#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API RigidBodyComponent final : public Hydra::World::IComponent<RigidBodyComponent, ComponentBits::RigidBody>{
		friend class Hydra::System::BulletPhysicsSystem;
		static constexpr const char* ShapeTypesStr[] = {
			"BOX_SHAPE_PROXYTYPE",
			"TRIANGLE_SHAPE_PROXYTYPE",
			"TETRAHEDRAL_SHAPE_PROXYTYPE",
			"CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE",
			"CONVEX_HULL_SHAPE_PROXYTYPE",
			"CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE",
			"CUSTOM_POLYHEDRAL_SHAPE_TYPE",
			"IMPLICIT_CONVEX_SHAPES_START_HERE",
			"SPHERE_SHAPE_PROXYTYPE",
			"MULTI_SPHERE_SHAPE_PROXYTYPE",
			"CAPSULE_SHAPE_PROXYTYPE",
			"CONE_SHAPE_PROXYTYPE",
			"CONVEX_SHAPE_PROXYTYPE",
			"CYLINDER_SHAPE_PROXYTYPE",
			"UNIFORM_SCALING_SHAPE_PROXYTYPE",
			"MINKOWSKI_SUM_SHAPE_PROXYTYPE",
			"MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE",
			"BOX_2D_SHAPE_PROXYTYPE",
			"CONVEX_2D_SHAPE_PROXYTYPE",
			"CUSTOM_CONVEX_SHAPE_TYPE",
			"CONCAVE_SHAPES_START_HERE",
			"TRIANGLE_MESH_SHAPE_PROXYTYPE",
			"SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE",
			"FAST_CONCAVE_MESH_PROXYTYPE",
			"TERRAIN_SHAPE_PROXYTYPE",
			"GIMPACT_SHAPE_PROXYTYPE",
			"MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE",
			"EMPTY_SHAPE_PROXYTYPE",
			"STATIC_PLANE_PROXYTYPE",
			"CUSTOM_CONCAVE_SHAPE_TYPE",
			"CONCAVE_SHAPES_END_HERE",
			"COMPOUND_SHAPE_PROXYTYPE",
			"SOFTBODY_SHAPE_PROXYTYPE",
			"HFFLUID_SHAPE_PROXYTYPE",
			"HFFLUID_BUOYANT_CONVEX_SHAPE_PROXYTYPE",
			"INVALID_SHAPE_PROXYTYPE",
			"MAX_BROADPHASE_COLLISION_TYPES"
		};

		enum class ActivationState : int{
			activeTag = 0,
			islandSleeping,
			wantsDeactivation,
			disableDeactivation,
			disableSimulation,

			MAX_COUNT
		};

		~RigidBodyComponent() final;

#define DEFAULT_PARAMS Hydra::System::BulletPhysicsSystem::CollisionTypes collType = Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_NOTHING, float mass = 0, float linearDamping = 0, float angularDamping = 0, float friction = 0, float rollingFriction = 0
		void createBox(const glm::vec3& halfExtents, const glm::vec3& offset, DEFAULT_PARAMS);
		void createStaticPlane(const glm::vec3& planeNormal, float planeConstant, DEFAULT_PARAMS);
		void createSphere(float radius, DEFAULT_PARAMS);
		//void createTriangleMesh(btStridingMeshInterface *meshInterface, bool useQuantizedAabbCompression, bool buildBvh=true, DEFAULT_PARAMS);
		void createCapsuleX(float radius, float height, DEFAULT_PARAMS);
		void createCapsuleY(float radius, float height, const glm::vec3& offset, DEFAULT_PARAMS);
		void createCapsuleZ(float radius, float height, DEFAULT_PARAMS);
		void createCylinderX(const glm::vec3& halfExtents, DEFAULT_PARAMS);
		void createCylinderY(const glm::vec3& halfExtents, DEFAULT_PARAMS);
		void createCylinderZ(const glm::vec3& halfExtents, DEFAULT_PARAMS);
#undef DEFAULT_PARAMS

		void setActivationState(ActivationState newState);
		void setAngularForce(glm::vec3 angularForce);
		void* getRigidBody();

		void refreshTransform();

		glm::vec3 getPosition(int childIndex = 0);
		glm::vec3 getHalfExtentScale() { return _halfExtents; }
		glm::quat getRotation();
		std::string getShapeString(int childIndex = 0);
		int getNumberOfChildren();

		inline const std::string type() const final { return "RigidBodyComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		Hydra::System::BulletPhysicsSystem::CollisionTypes doa = Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_NOTHING;
		struct Data;
		Data* _data = nullptr;
		glm::vec3 _halfExtents = glm::vec3(0.5f,0.5f,0.5f);
		Hydra::System::BulletPhysicsSystem* _handler = nullptr;
	};
};
