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

CameraComponent::~CameraComponent() {}

void CameraComponent::serialize(nlohmann::json& json) const {
	json["fov"] = fov;
	json["zNear"] = zNear;
	json["zFar"] = zFar;

	json["sensitivity"] = sensitivity;
	json["cameraYaw"] = cameraYaw;
	json["cameraPitch"] = cameraPitch;
	json["mouseControl"] = mouseControl;

	json["noClip"] = noClip;
	json["movementSpeed"] = movementSpeed;
	json["shiftMultiplier"] = shiftMultiplier;
}

void CameraComponent::deserialize(nlohmann::json& json) {
	fov = json.value<float>("fov", 90);
	zNear = json.value<float>("zNear", 0.1);
	zFar = json.value<float>("zFar", 75);

	sensitivity = json.value<float>("sensitivity", 0.003);
	cameraYaw = json.value<float>("cameraYaw", 0);
	cameraPitch = json.value<float>("cameraPitch", 0);
	mouseControl = json.value<bool>("mouseControl", true);

	noClip = json.value<bool>("noClip", false);
	movementSpeed = json.value<float>("movementSpeed", 10);
	shiftMultiplier = json.value<float>("shiftMultiplier", 5);
}

void CameraComponent::registerUI() {
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);

	ImGui::InputFloat("Aspect", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);

	ImGui::DragFloat("Sensitivity", &sensitivity);
	ImGui::DragFloat("Yaw", &cameraYaw, 0.0001);
	ImGui::DragFloat("Pitch", &cameraPitch, 0.0001);
	ImGui::Checkbox("Mouse Control", &mouseControl);

	ImGui::Checkbox("NoClip", &noClip);
	ImGui::InputFloat("Movement Speed", &movementSpeed);
	ImGui::InputFloat("Shift Multiplier", &shiftMultiplier);
}
