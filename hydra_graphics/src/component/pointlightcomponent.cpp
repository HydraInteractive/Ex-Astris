#include <hydra/component/pointlightcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

PointLightComponent::~PointLightComponent() {}

void PointLightComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position", { position.x, position.y, position.z } },
		{ "color", { color.x, color.y, color.z } },
		{ "constant", constant },
		{ "linear", linear },
		{ "quadratic", quadratic }
	};
}

void PointLightComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& c = json["color"];
	color = glm::vec3{ c[0].get<float>(), c[1].get<float>(), c[2].get<float>() };

	constant = json["constant"].get<float>();
	linear = json["linear"].get<float>();
	quadratic = json["quadratic"].get<float>();
}

void PointLightComponent::registerUI() {
	ImGui::DragFloat("Linear", &linear, 0.01f);
	ImGui::DragFloat("Constant", &constant, 0.0001f);
	ImGui::DragFloat("Quadratic", &quadratic, 0.0001f);
	ImGui::DragFloat3("color", glm::value_ptr(color), 0.01f);
}
