#include <hydra/component/lightcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

LightComponent::~LightComponent() {}

void LightComponent::serialize(nlohmann::json& json) const {
	json["colorX"] = color[0];
	json["colorY"] = color[1];
	json["colorZ"] = color[2];

	json["fov"] = fov;

	json["zNear"] = zNear;

	json["zFar"] = zFar;
}

void LightComponent::deserialize(nlohmann::json& json) {
	color[0] = json.value<float>("colorX", 0);
	color[1] = json.value<float>("colorY", 0);
	color[2] = json.value<float>("colorZ", 0);

	fov = json.value<float>("fov", 0);

	zNear = json.value<float>("zNear", 0);

	zFar = json.value<float>("zFar", 0);
}

void LightComponent::registerUI() {
	ImGui::LabelText("Direction", "%f, %f, %f", getDirVec().x, getDirVec().y, getDirVec().z);
	ImGui::DragFloat3("Color", glm::value_ptr(color), 0.01f);
	ImGui::DragFloat("FOV", &fov);
	ImGui::DragFloat("Z Near", &zNear, 0.001f);
	ImGui::DragFloat("Z Far", &zFar);
}
