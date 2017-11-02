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

#include <hydra/engine.hpp>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

PlayerSystem::PlayerSystem() {}
PlayerSystem::~PlayerSystem() {}

void PlayerSystem::tick(float delta) {
	const Uint8* keysArray = SDL_GetKeyboardState(nullptr);

	//Process PlayerComponent
	world::getEntitiesWithComponents<PlayerComponent, TransformComponent, CameraComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto player = entities[i]->getComponent<PlayerComponent>();
		auto transform = entities[i]->getComponent<TransformComponent>();
		auto camera = entities[i]->getComponent<CameraComponent>();
		auto weapon = player->getWeapon()->getComponent<Hydra::Component::WeaponComponent>();
		auto life = entities[i]->getComponent<Component::LifeComponent>();
		auto movement = entities[i]->getComponent<Component::MovementComponent>();
		auto soundFx = entities[i]->getComponent<SoundFxComponent>();
		auto perks = entities[i]->getComponent<PerkComponent>();

		//player->activeBuffs.onTick(life->maxHP, life->health);

		if (!life->statusCheck())
			player->isDead = true;


		glm::mat4 rotation = glm::mat4_cast(camera->orientation);

		{
			movement->velocity = glm::vec3{0};
			if (keysArray[SDL_SCANCODE_W])
				movement->velocity.z -= movement->movementSpeed;
			if (keysArray[SDL_SCANCODE_S])
				movement->velocity.z += movement->movementSpeed;

			if (keysArray[SDL_SCANCODE_A])
				movement->velocity.x -= movement->movementSpeed;
			if (keysArray[SDL_SCANCODE_D])
				movement->velocity.x += movement->movementSpeed;

			if (keysArray[SDL_SCANCODE_SPACE] && player->onGround){
				movement->acceleration.y += 6.0f;
				player->onGround = false;
			}

			if (keysArray[SDL_SCANCODE_F] && !player->prevKBFrameState[Keys::F]) {
				const glm::vec3 forward = glm::vec3(glm::vec4{0, 0, 1, 0} * rotation);
				auto abilitiesEntity = std::find_if(entities[i]->children.begin(), entities[i]->children.end(), [](Hydra::World::EntityID id) { return world::getEntity(id)->name == "Abilities"; });
				std::shared_ptr<Entity> e;
				if (abilitiesEntity == entities[i]->children.end())
					e = world::newEntity("Abilities", entities[i]);
				else
					e = world::getEntity(*abilitiesEntity);


				soundFx->soundsToPlay.push_back("assets/sounds/piano.wav");
				player->activeAbillies.useAbility(e.get(), transform->position, -forward);
			}

			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse) {
				const glm::vec3 forward = glm::vec3(glm::vec4{0, 0, 1, 0} * rotation);

				//TODO: Make pretty?
				glm::quat bulletOrientation = glm::angleAxis(-camera->cameraYaw, glm::vec3(0, 1, 0)) * (glm::angleAxis(-camera->cameraPitch, glm::vec3(1, 0, 0)));
				float bulletVelocity = 20.0f;

				weapon->shoot(transform->position, forward, bulletOrientation, bulletVelocity);
			}
		}

		movement->acceleration.y -= 10.0f * delta;
		glm::vec4 movementVector = glm::vec4(movement->velocity, 0) * rotation;
		movementVector.y = movement->acceleration.y;

		transform->position += glm::vec3(movementVector) * delta;

		if (transform->position.y < 0) {
			transform->position.y = 0;
			movement->acceleration.y = 0;
			player->onGround = true;
		}

		if (player->firstPerson)
			camera->position = transform->position;
		else
			camera->position = transform->position + glm::vec3(0, 3, 0) + glm::vec3(glm::vec4{-4, 0, 4, 0} * rotation);

		transform->rotation = camera->orientation;
		transform->dirty = true;

		auto wt = player->getWeapon()->getComponent<TransformComponent>();
		wt->position = transform->position + glm::vec3(glm::vec4{player->weaponOffset, 0} * rotation);
		wt->rotation = glm::normalize(glm::conjugate(camera->orientation) * glm::quat(glm::vec3(glm::radians(180.0f), 0, glm::radians(180.0f))));
		wt->dirty = true;

		player->prevKBFrameState[Keys::H] = keysArray[SDL_SCANCODE_H];
		player->prevKBFrameState[Keys::F] = keysArray[SDL_SCANCODE_F];
	}
}

void PlayerSystem::registerUI() {}
