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
	json = {
		{"position", {position.x, position.y, position.z}},
		{"orientation", {orientation.x, orientation.y, orientation.z, orientation.w}},
		{"fov", fov},
		{"zNear", zNear},
		{"zFar", zFar}
	};
}

void CameraComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	position = glm::vec3{pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};

	auto& ori = json["orientation"];
	orientation = glm::quat{ori[3].get<float>(), ori[0].get<float>(), ori[1].get<float>(), ori[2].get<float>()};

	fov = json["fov"].get<float>();
	zNear = json["zNear"].get<float>();
	zFar = json["zFar"].get<float>();
}

void CameraComponent::registerUI() {
	//TODO: Change if dirty flag is added!
	ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
	ImGui::DragFloat4("Orientation", glm::value_ptr(orientation), 0.01f);
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);

	ImGui::InputFloat("Aspect", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);

	ImGui::DragFloat("Sensitivity", &sensitivity);
	ImGui::DragFloat("Yaw", &cameraYaw);
	ImGui::DragFloat("Pitch", &cameraPitch);
	ImGui::Checkbox("Mouse Control", &mouseControl);
}
