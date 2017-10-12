#include <hydra/component/lightcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

LightComponent::~LightComponent() {}

void LightComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ position.x, position.y, position.z } },
		{ "direction", { direction.x, direction.y, direction.z } },
		{ "fov", fov },
		{ "zNear", zNear },
		{ "zFar", zFar }
	};
}

void LightComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& dir = json["direction"];
	direction = glm::vec3{ dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>() };

	fov = json["fov"].get<float>();
	zNear = json["zNear"].get<float>();
	zFar = json["zFar"].get<float>();
}

void LightComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
	ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f);
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);
}
