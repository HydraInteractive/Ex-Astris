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
#include <hydra/component/particlecomponent.hpp>
#define BIT(x) (1 << (x))

using namespace Hydra::Component;

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
			COLL_NOTHING = BIT(0),
			COLL_WALL = BIT(1),
			COLL_PLAYER = BIT(2),
			COLL_ENEMY = BIT(3),
			COLL_PLAYER_PROJECTILE = BIT(4),
			COLL_ENEMY_PROJECTILE = BIT(5),
			COLL_MISC_OBJECT = BIT(6),
			COLL_PICKUP_OBJECT = BIT(7),
			COLL_FLOOR = BIT(8)
		};

		enum CollisionCondition : std::underlying_type<CollisionTypes>::type {
			playerCollidesWith = COLL_WALL | COLL_ENEMY | COLL_MISC_OBJECT | COLL_ENEMY_PROJECTILE | COLL_PICKUP_OBJECT | COLL_NOTHING | COLL_FLOOR,
			enemyCollidesWith = COLL_WALL | COLL_PLAYER | COLL_MISC_OBJECT | COLL_PLAYER_PROJECTILE | COLL_FLOOR,
			wallCollidesWith = COLL_MISC_OBJECT | COLL_PLAYER | COLL_ENEMY | COLL_ENEMY_PROJECTILE | COLL_PLAYER_PROJECTILE | COLL_NOTHING,
			enemyProjCollidesWith = COLL_PLAYER | COLL_WALL | COLL_MISC_OBJECT | COLL_FLOOR,
			playerProjCollidesWith = COLL_ENEMY | COLL_WALL | COLL_MISC_OBJECT | COLL_FLOOR,
			miscObjectCollidesWith = COLL_WALL | COLL_ENEMY | COLL_PLAYER | COLL_ENEMY_PROJECTILE | COLL_PLAYER_PROJECTILE | COLL_FLOOR,
			pickupObjectCollidesWith = COLL_PLAYER,
			floorCollidesWith = COLL_MISC_OBJECT | COLL_PLAYER | COLL_ENEMY | COLL_ENEMY_PROJECTILE | COLL_PLAYER_PROJECTILE | COLL_NOTHING
		};

		BulletPhysicsSystem();
		~BulletPhysicsSystem() final;

		void enable(RigidBodyComponent* component);

		void disable(RigidBodyComponent* component);

		void enable(GhostObjectComponent* component);
		void disable(GhostObjectComponent* component);

		void* rayTestFromTo(const glm::vec3& from, const glm::vec3& to);

		void tick(float delta) final;

		inline const std::string type() const final { return "BulletPhysicsSystem"; }
		void registerUI() final;

	private:
		void _spawnParticleEmitterAt(const glm::vec3& pos, const glm::vec3& normal, const Hydra::Component::ParticleComponent::ParticleTexture& effect);
		void _spawnText(const glm::vec3& pos, const std::string& text);
		void _addPickUp(PickUpComponent* puc, PerkComponent* pec);
		struct Data;
		Data* _data;
	};
}
