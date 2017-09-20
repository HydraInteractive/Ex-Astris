// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A camera component that generates view and projection matrices.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/component/cameracomponent.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

CameraComponent::CameraComponent(IEntity* entity) : IComponent(entity), _renderTarget(nullptr) {}
CameraComponent::CameraComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position) : IComponent(entity), _renderTarget(renderTarget), _position(position) {}

CameraComponent::~CameraComponent() {}

void CameraComponent::tick(TickAction action) {
	_position += glm::vec3{0, 0, 0};

	int mouseX, mouseY;
	if (_mouseControl && SDL_GetRelativeMouseState(&mouseX, &mouseY) == SDL_BUTTON(3)) {
		_cameraYaw += mouseX * _sensitivity;
		_cameraPitch -= mouseY *_sensitivity;

		if (_cameraPitch > glm::radians(89.0f)){
			_cameraPitch = glm::radians(89.0f);
		}
		else if(_cameraPitch < glm::radians(-89.0f)){
			_cameraPitch = glm::radians(-89.0f);
		}
	}

	glm::quat qPitch = glm::angleAxis(_cameraPitch, glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(_cameraYaw, glm::vec3(0, 1, 0));

	_orientation = qPitch * qYaw;
	_orientation = glm::normalize(_orientation);

}

void CameraComponent::translate(const glm::vec3& transform) {
	_position += transform * _orientation;
}

void CameraComponent::rotation(float angle, const glm::vec3& axis) {
	_orientation *= glm::angleAxis(angle, axis * _orientation);
}

CameraComponent& CameraComponent::yaw(float angle) { rotation(angle, {0, 1, 0}); return *this; }
CameraComponent& CameraComponent::pitch(float angle) { rotation(angle, {1, 0, 0}); return *this; }
CameraComponent& CameraComponent::roll(float angle) { rotation(angle, {0, 0, 1}); return *this; }

void Hydra::Component::CameraComponent::setPosition(const glm::vec3 & position) {
	_position = position;
}

void CameraComponent::serialize(nlohmann::json& json) const {
	json = {
		{"position", {_position.x, _position.y, _position.z}},
		{"orientation", {_orientation.x, _orientation.y, _orientation.z, _orientation.w}},
		{"fov", _fov},
		{"zNear", _zNear},
		{"zFar", _zFar}
	};
}

void CameraComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};

	auto& orientation = json["orientation"];
	_orientation = glm::quat{orientation[0].get<float>(), orientation[1].get<float>(), orientation[2].get<float>(), orientation[3].get<float>()};

	_fov = json["fov"].get<float>();
	_zNear = json["zNear"].get<float>();
	_zFar = json["zFar"].get<float>();
}

void CameraComponent::registerUI() {
	//TODO: Change if dirty flag is added!
	ImGui::DragFloat3("Position", glm::value_ptr(_position), 0.01f);
	ImGui::DragFloat4("Orientation", glm::value_ptr(_orientation), 0.01f);
	ImGui::DragFloat("FOV", &_fov);
	ImGui::DragFloat("Z Near", &_zNear, 0.001f);
	ImGui::DragFloat("Z Far", &_zFar);
	if (_renderTarget) {
		float aspect = (_renderTarget->getSize().x*1.0f) / _renderTarget->getSize().y;
		ImGui::InputFloat("Aspect", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);
		ImGui::Checkbox("Mouse Control", &_mouseControl);
	}
}
