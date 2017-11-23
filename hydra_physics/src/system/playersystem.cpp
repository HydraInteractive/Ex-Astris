#include <hydra/system/playersystem.hpp>

#include <imgui/imgui.h>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

#include <btBulletDynamicsCommon.h>

#include <hydra/engine.hpp>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

PlayerSystem::PlayerSystem() {}
PlayerSystem::~PlayerSystem() {}

float lerp(float a, float b, float f) {
	return a + f * (b - a);
}

void PlayerSystem::tick(float delta) {
	const Uint8* keysArray = SDL_GetKeyboardState(nullptr);

	//Process PlayerComponent
	world::getEntitiesWithComponents<PlayerComponent, TransformComponent, CameraComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto player = entities[i]->getComponent<PlayerComponent>();
		auto transform = entities[i]->getComponent<TransformComponent>();
		auto camera = entities[i]->getComponent<CameraComponent>();
		auto weapon = player->getWeapon()->getComponent<Hydra::Component::WeaponComponent>();
		auto weaponMesh = player->getWeapon()->getComponent<Hydra::Component::MeshComponent>();
		auto life = entities[i]->getComponent<Component::LifeComponent>();
		auto movement = entities[i]->getComponent<Component::MovementComponent>();
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();
		auto perks = entities[i]->getComponent<PerkComponent>();
		auto rbc = static_cast<btRigidBody*>(entities[i]->getComponent<RigidBodyComponent>()->getRigidBody());

		glm::mat4 rotation = glm::mat4_cast(transform->rotation);
		movement->direction = -glm::vec3(glm::vec4{ 0, 0, 1, 0 } *rotation);

		{
			glm::vec3 forward = glm::normalize(glm::vec3(movement->direction.x, 0, movement->direction.z));			

			glm::vec3 rightDir = glm::vec3(glm::vec4{ 1, 0, 0, 0 } *rotation);
			glm::vec3 right = glm::normalize(glm::vec3(rightDir.x, 0, rightDir.z));

			if (keysArray[SDL_SCANCODE_W])
				movement->velocity += movement->movementSpeed * forward * delta;

			if (keysArray[SDL_SCANCODE_R])
				weapon->_isReloading = true;

			if (keysArray[SDL_SCANCODE_S]) {
				movement->velocity -= movement->movementSpeed * forward * delta;
			}

			if (keysArray[SDL_SCANCODE_A])
				movement->velocity -= movement->movementSpeed * right * delta;

			if (keysArray[SDL_SCANCODE_D])
				movement->velocity += movement->movementSpeed * right * delta;

			if (keysArray[SDL_SCANCODE_SPACE] && player->onGround) {
				rbc->applyCentralForce(btVector3(0, 12000, 0));
				player->onGround = false;
			}
			if (movement->velocity.x != 0 || movement->velocity.y != 0 || movement->velocity.z != 0)
				weaponMesh->animationIndex = 1;
			else
				weaponMesh->animationIndex = 0;

			if (camera->mouseControl && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				
				glm::vec3 dirPlusRange(transform->position + (movement->direction*50.0f));
				if (static_cast<btCollisionWorld::ClosestRayResultCallback*>(static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem())->rayTestFromTo(transform->position, dirPlusRange))->hasHit())
					printf("YEAH BOI\n");
				
				//TODO: Make pretty?
				glm::quat bulletOrientation = glm::angleAxis(-camera->cameraYaw, glm::vec3(0, 1, 0)) * (glm::angleAxis(-camera->cameraPitch, glm::vec3(1, 0, 0)));

				//Changed to see if modell is correct, original bulletVelocity was 300 (A little too fast imo, das me tho)
				float bulletVelocity = 20;
				if (!weapon->_isReloading)
					if (weapon->shoot(glm::vec3(transform->position + movement->direction * glm::vec3(5)) , movement->direction, bulletOrientation, bulletVelocity, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE)) {
						float rn = 500;//rand() % 1000;
						rn /= 10000;

						rn *= 0.8;
						weapon->_dpitch -= rn;
						rn = rand() % 900 + 100;
						rn /= 10000;
						rn *= 0.8;
						//if (rand() % 2 == 1)
						//	dyaw += rn/3;
						//else
						//	dyaw -= rn/3;
						weaponMesh->animationIndex = 2;
						if (weapon->currmagammo == 0)
							weapon->_isReloading = true;
					}
			}
		}
		if (!keysArray[SDL_SCANCODE_W]
			&& !keysArray[SDL_SCANCODE_S]
			&& !keysArray[SDL_SCANCODE_A]
			&& !keysArray[SDL_SCANCODE_D]) {
			movement->velocity *= delta;
		}

		float speed = glm::length(movement->velocity);
		if (speed > 10)
			movement->velocity *= 10 / speed;

		if (weapon->_isReloading)
			weapon->_isReloading = weapon->reload(delta);

		float& yaw = camera->cameraYaw;
		float& pitch = camera->cameraPitch;
		yaw = lerp(yaw, (yaw + weapon->_dyaw), 0.5);
		pitch = lerp(pitch, (pitch + weapon->_dpitch), 0.5);
		weapon->_dyaw /= 2;
		weapon->_dpitch /= 2;

		btVector3 vel = rbc->getLinearVelocity();
		rbc->setLinearVelocity(btVector3(movement->velocity.x,vel.y(),movement->velocity.z));

		transform->dirty = true;

		auto wt = player->getWeapon()->getComponent<TransformComponent>();
		wt->position = transform->position + glm::vec3(glm::vec4{player->weaponOffset, 0} * rotation);
		wt->rotation = glm::normalize(glm::conjugate(transform->rotation) * glm::quat(glm::vec3(glm::radians(180.0f), 0, glm::radians(180.0f))));
		wt->dirty = true;
	}

	entities.clear();
}

void PlayerSystem::registerUI() {}
