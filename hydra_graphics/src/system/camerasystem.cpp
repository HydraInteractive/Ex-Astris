#include <hydra/system/camerasystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/EditorCameraComponent.hpp>
#include <algorithm>

using namespace Hydra::System;

CameraSystem::CameraSystem() {}
CameraSystem::~CameraSystem() {}

void CameraSystem::tick(float delta) {
	using world = Hydra::World::World;

	// Collect data
	int mouseX = 0;
	int mouseY = 0;
	glm::vec3 velocity = glm::vec3{0, 0, 0};
	bool multiplier = false;

	if ((SDL_GetRelativeMouseState(&mouseX, &mouseY) & SDL_BUTTON(3)) == 0)
		mouseX = mouseY = 0;


	{
		const Uint8* keysArray = SDL_GetKeyboardState(nullptr);

		if (keysArray[SDL_SCANCODE_W])
			velocity.z -= 1;
		if (keysArray[SDL_SCANCODE_S])
			velocity.z += 1;
		if (keysArray[SDL_SCANCODE_A])
			velocity.x -= 1;
		if (keysArray[SDL_SCANCODE_D])
			velocity.x += 1;

		multiplier = keysArray[SDL_SCANCODE_LSHIFT];//velocity *= ;
	}

	//Process CameraComponent
	world::getEntitiesWithComponents<Hydra::Component::CameraComponent>(entities);

#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto cc = entities[i]->getComponent<Hydra::Component::CameraComponent>();

		if (cc->mouseControl) {
			cc->cameraYaw = cc->cameraYaw + mouseX * cc->sensitivity; // std::min(std::max(cc->cameraYaw + mouseX * cc->sensitivity, glm::radians(-89.9999f)), glm::radians(89.9999f));
			cc->cameraPitch = std::min(std::max(cc->cameraPitch + mouseY * cc->sensitivity, glm::radians(-89.9999f)), glm::radians(89.9999f));
		}

		glm::quat qPitch = glm::angleAxis(cc->cameraPitch, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(cc->cameraYaw, glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1));

		cc->orientation = qPitch * qYaw * qRoll;
		cc->orientation = glm::normalize(cc->orientation);
	}

	//Process EditorCameraComponent
	world::getEntitiesWithComponents<Hydra::Component::EditorCameraComponent>(entities);
#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto ec = entities[i]->getComponent<Hydra::Component::EditorCameraComponent>();

		ec->cameraYaw = ec->cameraYaw + mouseX * ec->sensitivity; //std::min(std::max(ec->cameraYaw + mouseX * ec->sensitivity, glm::radians(-89.9999f)), glm::radians(89.9999f));
		ec->cameraPitch = std::min(std::max(ec->cameraPitch + mouseY * ec->sensitivity, glm::radians(-89.9999f)), glm::radians(89.9999f));

		glm::quat qPitch = glm::angleAxis(ec->cameraPitch, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(ec->cameraYaw, glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1));

		ec->orientation = qPitch * qYaw * qRoll;
		ec->orientation = glm::normalize(ec->orientation);

		const glm::mat4 viewMat = ec->getViewMatrix();
		ec->position += glm::vec3(glm::vec4(velocity * ec->movementSpeed * (multiplier ? ec->shiftMultiplier : 1), 1.0f) * viewMat) * delta;
	}
}

void CameraSystem::registerUI() {}
