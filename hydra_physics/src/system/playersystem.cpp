#include <hydra/system/playersystem.hpp>

#include <hydra/component/playercomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>

using namespace Hydra::Component;

PlayerSystem::~PlayerSystem() {}

void PlayerSystem::tick(float delta) {
	using world = Hydra::World::World;
	static std::vector<std::shared_ptr<Entity>> entities;

	//Process PlayerComponent
	world::getEntitiesWithComponents<PlayerComponent, TransformComponent, CameraComponent>(entities);
	#pragma omp parallel for
	for (size_t i = 0; i < entities.size(); i++) {
		auto player = entities[i]->getComponent<PlayerComponent>();
		auto transform = entities[i]->getComponent<TransformComponent>();
		auto camera = entities[i]->getComponent<CameraComponent>();

		player->activeBuffs.onTick(player->maxHealth, player->health);

		if (player->health <= 0)
			player->dead = true;

		auto weapon = player->getWeapon()->getComponent<Component::WeaponComponent>();

		glm::mat4 rotation = glm::mat4_cast(camera->getOrientation());

		{
			const Uint8* keysArray = SDL_GetKeyboardState(&keysArrayLength);
			player->velocity = glm::vec3{0};
			if (keysArray[SDL_SCANCODE_W])
				player->velocity.z -= player->movementSpeed;
			if (keysArray[SDL_SCANCODE_S])
				player->velocity.z += player->movementSpeed;

			if (keysArray[SDL_SCANCODE_A])
				player->velocity.x -= player->movementSpeed;
			if (keysArray[SDL_SCANCODE_D])
				player->velocity.x += player->movementSpeed;

			if (keysArray[SDL_SCANCODE_SPACE] && player->onGround){
				player->acceleration.y += 6.0f;
				player->onGround = false;
			}
			if (keysArray[SDL_SCANCODE_H] && !lastKeysArray[SDL_SCANCODE_H])
				{
					upgradeHealth();
				}

			if (keysArray[SDL_SCANCODE_F] && !lastKeysArray[SDL_SCANCODE_F]) {
				auto& player = entity->getChildren();
				const glm::vec3 forward = glm::vec3(glm::vec4{0, 0, 1, 0} * rotation);
				auto abilitiesEntity = std::find_if(player.begin(), player.end(), [](const std::shared_ptr<IEntity>& e) { return e->getName() == "Abilities"; });
				player->activeAbillies.useAbility(abilitiesEntity->get(), player->position, -forward);
			}

			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse) {
				const glm::vec3 forward = glm::vec3(glm::vec4{0, 0, 1, 0} * rotation);

				//TODO: Make pretty?
				glm::quat bulletOrientation = glm::angleAxis(-camera->getYaw(), glm::vec3(0, 1, 0)) * (glm::angleAxis(-camera->getPitch(), glm::vec3(1, 0, 0)));
				float bulletVelocity = 1.0f;
				player->activeBuffs.onAttack(bulletVelocity);

				weapon->shoot(player->position, forward, bulletOrientation, bulletVelocity);
			}
			for (int i = 0; i < keysArrayLength; i++)
				lastKeysArray[i] = keysArray[i];
		}

		player->acceleration.y -= 10.0f * delta;
		glm::vec4 movementVector = glm::vec4(player->velocity, 0) * rotation;
		movementVector.y = player->acceleration.y;

		player->position += glm::vec3(movementVector) * delta;

		if (player->position.y < 0) {
			player->position.y = 0;
			player->acceleration.y = 0;
			player->onGround = true;
		}

		if (player->firstPerson)
			camera->setPosition(player->position);
		else
			camera->setPosition(player->position + glm::vec3(0, 3, 0) + glm::vec3(glm::vec4{-4, 0, 4, 0} * rotation));

		transform->setPosition(player->position);
		transform->setRotation(camera->getOrientation());

		getWeapon()->getComponent<TransformComponent>()->setPosition(player->position + glm::vec3(glm::vec4{_weaponOffset, 0} * rotation));
		getWeapon()->getComponent<TransformComponent>()->setRotation(glm::normalize(glm::conjugate(camera->getOrientation()) * glm::quat(glm::vec3(glm::radians(180.0f), 0, glm::radians(180.0f)))));
	}
}

void PlayerSystem::registerUI() {}
