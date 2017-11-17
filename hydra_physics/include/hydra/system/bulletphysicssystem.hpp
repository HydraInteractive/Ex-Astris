/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#define BIT(x) (1 << (x))

namespace Hydra::System {
	class HYDRA_PHYSICS_API BulletPhysicsSystem final : public Hydra::World::ISystem {
	public:
		enum class CollisionShape {
			Box,
			StaticPlane,
			Sphere,
			//TriangleMesh, // btBvhTriangleMesh
			CapsuleX,
			CapsuleY, // btCapsuleShape
			CapsuleZ,
			CylinderX,
			CylinderY, // btCylinderShape
			CylinderZ
		};

		enum CollisionTypes : const int{
			COLL_NOTHING = 0,
			COLL_WALL = BIT(1),
			COLL_PLAYER = BIT(2),
			COLL_ENEMY = BIT(3),
			COLL_PLAYER_PROJECTILE = BIT(4),
			COLL_ENEMY_PROJECTILE = BIT(5),
			COLL_MISC_OBJECT = BIT(6),
			COLL_PICKUP_OBJECT = BIT(7)
		};

		enum CollisionCondition : std::underlying_type<CollisionTypes>::type {
			playerCollidesWith = COLL_WALL | COLL_ENEMY | COLL_MISC_OBJECT | COLL_ENEMY_PROJECTILE | COLL_PICKUP_OBJECT,
			enemyCollidesWith = COLL_WALL | COLL_PLAYER | COLL_MISC_OBJECT | COLL_PLAYER_PROJECTILE,
			wallCollidesWith = COLL_MISC_OBJECT | COLL_PLAYER | COLL_ENEMY | COLL_ENEMY_PROJECTILE | COLL_PLAYER_PROJECTILE,
			enemyProjCollidesWith = COLL_PLAYER | COLL_WALL | COLL_MISC_OBJECT,
			playerProjCollidesWith = COLL_ENEMY | COLL_WALL | COLL_MISC_OBJECT,
			miscObjectCollidesWith = COLL_WALL | COLL_ENEMY | COLL_PLAYER | COLL_ENEMY_PROJECTILE | COLL_PLAYER_PROJECTILE,
			pickupObjectCollidesWith = COLL_PLAYER
		};

		BulletPhysicsSystem();
		~BulletPhysicsSystem() final;

		void enable(Hydra::Component::RigidBodyComponent* component);

		void disable(Hydra::Component::RigidBodyComponent* component);

		void tick(float delta) final;

		inline const std::string type() const final { return "BulletPhysicsSystem"; }
		void registerUI() final;

	private:
		void _spawnParticleEmitterAt(const glm::vec3& pos, const glm::vec3& normal);
		void _spawnDamageText(const glm::vec3& pos, const float& damage);
		void _addPickUp(Hydra::Component::PickUpComponent* puc, Hydra::Component::PerkComponent* pec);
		struct Data;
		Data* _data;
	};
}
