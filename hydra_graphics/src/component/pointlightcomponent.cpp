#include <hydra/component/pointlightcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

PointLightComponent::~PointLightComponent() {}

void PointLightComponent::serialize(nlohmann::json& json) const {
	json["colorX"] = color[0];
	json["colorY"] = color[1];
	json["colorZ"] = color[2];

	json["constant"] = constant;

	json["linear"] = linear;

	json["quadratic"] = quadratic;
}

void PointLightComponent::deserialize(nlohmann::json& json) {
	color[0] = json.value<float>("colorX", 0);
	color[1] = json.value<float>("colorY", 0);
	color[2] = json.value<float>("colorZ", 0);

	constant = json.value<float>("constant", 0);

	linear = json.value<float>("linear", 0);

	quadratic = json.value<float>("quadratic", 0);
}

void PointLightComponent::registerUI() {
	ImGui::DragFloat("Linear", &linear, 0.01f);
	ImGui::DragFloat("Constant", &constant, 0.0001f);
	ImGui::DragFloat("Quadratic", &quadratic, 0.0001f);
	ImGui::DragFloat3("color", glm::value_ptr(color), 0.01f);
}
