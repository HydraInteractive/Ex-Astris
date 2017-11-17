#include <hydra/system/camerasystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/cameracomponent.hpp>
#include <algorithm>

#include <hydra/engine.hpp>
#include <hydra/view/view.hpp>
#include <SDL2/SDL.h>

#include <imgui/imguizmo.h>

using namespace Hydra::System;

#define ANG2RAD (3.14159265358979323846/180.0)

CameraSystem::CameraSystem() {}
CameraSystem::~CameraSystem() {}

void CameraSystem::tick(float delta) {
	using world = Hydra::World::World;
	Hydra::View::IView* view = Hydra::IEngine::getInstance()->getView();
	auto viewSize = view->getSize();

	// Collect data
	glm::vec3 velocity = glm::vec3{0, 0, 0};
	bool multiplier = false;

	bool toggleMouse = false;

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

		static bool wasCtrlDown = false;
		if (keysArray[SDL_SCANCODE_LCTRL] && !wasCtrlDown)
			toggleMouse = wasCtrlDown = true;
		else if (!keysArray[SDL_SCANCODE_LCTRL] && wasCtrlDown)
			wasCtrlDown = false;

		multiplier = keysArray[SDL_SCANCODE_LSHIFT];
	}

	//Process CameraComponent
	world::getEntitiesWithComponents<Hydra::Component::CameraComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto cc = entities[i]->getComponent<Hydra::Component::CameraComponent>();
		if (i == 0) {
			if (toggleMouse) {
				cc->mouseControl = !cc->mouseControl;
				SDL_WarpMouseInWindow(static_cast<SDL_Window*>(view->getHandler()), viewSize.x / 2, viewSize.y / 2);
			} else if (cc->mouseControl) {
				glm::ivec2 mousePos;
				SDL_GetMouseState(&mousePos.x, &mousePos.y);
				mousePos -= viewSize / 2;
				SDL_WarpMouseInWindow(static_cast<SDL_Window*>(view->getHandler()), viewSize.x / 2, viewSize.y / 2);

				cc->cameraYaw = cc->cameraYaw + mousePos.x * cc->sensitivity;
				cc->cameraPitch = std::min(std::max(cc->cameraPitch + mousePos.y * cc->sensitivity, glm::radians(-89.9999f)), glm::radians(89.9999f));
			}
			SDL_ShowCursor(cc->mouseControl ? SDL_DISABLE : SDL_ENABLE);
			ImGuizmo::Enable(!cc->mouseControl);
		}

		glm::quat qPitch = glm::angleAxis(cc->cameraPitch, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(cc->cameraYaw, glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1));

		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		t->rotation = glm::normalize(qPitch * qYaw * qRoll);

		if (cc->noClip) {
			const glm::mat4 viewMat = cc->getViewMatrix();
			t->position += glm::vec3(glm::vec4(velocity * cc->movementSpeed * (multiplier ? cc->shiftMultiplier : 1), 1.0f) * viewMat) * delta;
		}
	}

	entities.clear();
}

void CameraSystem::setCamInternals(Hydra::Component::CameraComponent& cc) {
	cc.tang = (float)tan(ANG2RAD * cc.fov * 0.5);
	cc.nh = cc.zNear * cc.tang;
	cc.nw = cc.nh * cc.aspect;
	cc.fh = cc.zFar * cc.tang;
	cc.fw = cc.fh * cc.aspect;
}

void CameraSystem::setCamDef(const glm::vec3& cPos, const glm::vec3& cDir, const glm::vec3& up, const glm::vec3& right, Hydra::Component::CameraComponent& cc) {
	glm::vec3 dir, nc, fc, X, Y, Z;

	Z = cPos - cDir;
	Z = glm::normalize(Z);

	X = up * Z;
	X = glm::normalize(X);

	Y = Z * X;

	nc = cPos - cDir * cc.zNear;
	fc = cPos - cDir * cc.zFar;

	//near plane
	cc.ntl = nc + up * cc.nh - right * cc.nw;
	cc.ntr = nc + up * cc.nh + right * cc.nw;
	cc.nbl = nc - up * cc.nh - right * cc.nw;
	cc.nbr = nc - up * cc.nh + right * cc.nw;

	//far plane
	cc.ftl = fc + up * cc.fh - right * cc.fw;
	cc.ftr = fc + up * cc.fh + right * cc.fw;
	cc.fbl = fc - up * cc.fh - right * cc.fw;
	cc.fbr = fc - up * cc.fh + right * cc.fw;

	cc.pl[cc.TOP].set3Points(cc.ntr, cc.ntl, cc.ftl);
	cc.pl[cc.BOTTOM].set3Points(cc.nbl, cc.nbr, cc.fbr);
	cc.pl[cc.LEFT].set3Points(cc.ntl, cc.nbl, cc.fbl);
	cc.pl[cc.RIGHT].set3Points(cc.nbr, cc.ntr, cc.fbr);
	cc.pl[cc.NEARP].set3Points(cc.ntl, cc.ntr, cc.nbr);
	cc.pl[cc.FARP].set3Points(cc.ftr, cc.ftl, cc.fbl);

}

CameraSystem::FrustrumCheck CameraSystem::sphereInFrustum(const glm::vec3& p, float radius, Hydra::Component::CameraComponent& cc) {
	float distance = 0;
	CameraSystem::FrustrumCheck result = CameraSystem::FrustrumCheck::inside;
	for (int i = 0; i < 6; i++) {
		distance = cc.pl[i].distance(p);
		if (distance < -radius)
			return CameraSystem::FrustrumCheck::outside;
		else if (distance < radius)
			result = CameraSystem::FrustrumCheck::intersect;
	}
	return result;
}


void CameraSystem::registerUI() {}
