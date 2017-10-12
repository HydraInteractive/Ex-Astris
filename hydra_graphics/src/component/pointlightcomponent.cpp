#include "hydra/component/pointlightcomponent.hpp"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

PointLightComponent::PointLightComponent(IEntity* entity) : IComponent(entity) {
	_position = glm::vec3(0, 0, 0);
	_color = glm::vec3(1, 0, 0);
	_constant = 1;
	_linear = 0.045;
	_quadratic = 0.0075;
}

PointLightComponent::PointLightComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position) : IComponent(entity), _position(position),
_color(1, 0, 0), _constant(1), _linear(0.045), _quadratic(0.0075) {

}

PointLightComponent::~PointLightComponent() {

}

void PointLightComponent::tick(TickAction action, float delta) {
	this->_position = this->entity->getComponent<Hydra::Component::TransformComponent>()->getPosition();
}

void PointLightComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "color",{ _color.x, _color.y, _color.z } },
		{ "constant", _constant },
		{ "linear", _linear },
		{ "quadratic", _quadratic }
	};
}

void PointLightComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	_constant = json["constant"].get<float>();
	_linear = json["linear"].get<float>();
	_quadratic = json["quadratic"].get<float>();

	auto& color = json["color"];
	_color = glm::vec3{ color[0].get<float>(), color[1].get<float>(), color[2].get<float>() };
}

void PointLightComponent::registerUI() {
	ImGui::DragFloat("Linear", &_linear, 0.01f);
	ImGui::DragFloat("Constant", &_constant, 0.0001f);
	ImGui::DragFloat("Quadratic", &_quadratic, 0.0001f);
	ImGui::DragFloat3("color", glm::value_ptr(_color), 0.01f);
}