#include <hydra/component/freecameracomponent.hpp>

Hydra::Component::FreeCameraComponent::~FreeCameraComponent() {}

void Hydra::Component::FreeCameraComponent::serialize(nlohmann::json & json) const {
	json = {
		{ "fov", fov },
		{ "zNear", zNear },
		{ "zFar", zFar },
		{ "movementSpeed", movementSpeed },
		{ "shiftMultiplier", shiftMultiplier }
	};
}

void Hydra::Component::FreeCameraComponent::deserialize(nlohmann::json & json) {
	fov = json["fov"].get<float>();
	zNear = json["zNear"].get<float>();
	zFar = json["zFar"].get<float>();
	movementSpeed = json["movementSpeed"].get<float>();
	shiftMultiplier = json["shiftMultiplier"].get<float>();
}

void Hydra::Component::FreeCameraComponent::registerUI() {
	ImGui::InputFloat("Movement Speed", &movementSpeed);
	ImGui::InputFloat("Shift Multiplier", &shiftMultiplier);
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);
	ImGui::InputFloat("Aspect Ratio", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);

}
