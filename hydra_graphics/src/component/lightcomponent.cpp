#include "hydra/component/lightcomponent.hpp"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

LightComponent::LightComponent(IEntity* entity) : IComponent(entity) {
	_position = glm::vec3(0, 0, 0);
	_color = glm::vec3(1, 0, 0);
}

LightComponent::LightComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position) : IComponent(entity), _position(position),
_color(1,0,0){

}

LightComponent::~LightComponent() {

}

void LightComponent::setPosition(const glm::vec3& position) {
	_position = position;
}

void LightComponent::setDirection(const glm::vec3& direction) {
	_direction = direction;
}

void LightComponent::translate(const glm::vec3& transform) {
	_position += transform;
}

void LightComponent::tick(TickAction action, float delta) {
	this->_position = this->entity->getComponent<Hydra::Component::TransformComponent>()->getPosition();
}

void LightComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		//{ "orientation",{ _orientation.x, _orientation.y, _orientation.z, _orientation.w } },
		{ "direction", { _direction.x, _direction.y, _direction.z } },
		{ "fov", _fov },
		{ "zNear", _zNear },
		{ "zFar", _zFar },
		{ "color", _color}
	};
}

void LightComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };
	
	//auto& orientation = json["orientation"];
	//_orientation = glm::quat{ orientation[0].get<float>(), orientation[1].get<float>(), orientation[2].get<float>(), orientation[3].get<float>() };
	
	auto& direction = json["direction"];
	_direction = glm::vec3{ direction[0].get<float>(), direction[1].get<float>(), direction[2].get<float>() };

	_fov = json["fov"].get<float>();
	_zNear = json["zNear"].get<float>();
	_zFar = json["zFar"].get<float>();

	auto& color = json["color"];
	_color = glm::vec3{color[0].get<float>(), color[1].get<float>(), color[2].get<float>() };
}

void LightComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position), 0.01f);
	ImGui::DragFloat3("Direction", glm::value_ptr(_direction), 0.01f);
	ImGui::DragFloat("FOV", &_fov);
	ImGui::DragFloat("Z Near", &_zNear, 0.001f);
	ImGui::DragFloat("Z Far", &_zFar);
	ImGui::DragFloat3("Color", glm::value_ptr(_color), 0.01f);
	
	//float aspect = (_renderTarget->getSize().x*1.0f) / _renderTarget->getSize().y;
	//ImGui::InputFloat("Aspect", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);
}
