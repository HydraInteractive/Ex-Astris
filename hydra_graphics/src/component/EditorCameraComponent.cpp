#include <hydra/component/EditorCameraComponent.hpp>

Hydra::Component::EditorCameraComponent::~EditorCameraComponent() {}

void Hydra::Component::EditorCameraComponent::serialize(nlohmann::json & json) const {
	json = {
		{ "position", { position.x, position.y, position.z } },
		{ "orientation", { orientation.x, orientation.y, orientation.z, orientation.w } },
		{ "fov", fov },
		{ "zNear", zNear },
		{ "zFar", zFar },
		{ "movementSpeed", movementSpeed },
		{ "shiftMultiplier", shiftMultiplier }
	};
}

void Hydra::Component::EditorCameraComponent::deserialize(nlohmann::json & json) {
	auto& pos = json["position"];
	position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& ori = json["orientation"];
	orientation = glm::quat{ori[3].get<float>(), ori[0].get<float>(), ori[1].get<float>(), ori[2].get<float>()};

	fov = json["fov"].get<float>();
	zNear = json["zNear"].get<float>();
	zFar = json["zFar"].get<float>();
	movementSpeed = json["movementSpeed"].get<float>();
	shiftMultiplier = json["shiftMultiplier"].get<float>();
}

void Hydra::Component::EditorCameraComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
	ImGui::InputFloat("Movement Speed", &movementSpeed);
	ImGui::InputFloat("Shift Multiplier", &shiftMultiplier);
	ImGui::DragFloat4("Orientation", glm::value_ptr(orientation), 0.01f);
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);

	float aspect = (renderTarget->getSize().x*1.0f) / renderTarget->getSize().y;
	ImGui::InputFloat("Aspect Ratio", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);

}
