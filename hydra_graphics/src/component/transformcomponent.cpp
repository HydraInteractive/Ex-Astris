// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A transform component that specifies that the entity has an position inside the world.
 * It contains a DrawObject instance from the renderer, to be able to update its location.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/component/transformcomponent.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

#include <hydra/engine.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

TransformComponent::TransformComponent(IEntity* entity, const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) : IComponent(entity), _drawObject(entity->getDrawObject()), _dirty(true), _position(position), _scale(scale), _rotation(rotation) {
	_drawObject->refCounter++;
}
TransformComponent::~TransformComponent() {
	_drawObject->modelMatrix = glm::mat4(1);
	_drawObject->refCounter--;
}

void TransformComponent::tick(TickAction action) {
	// assert(action == TickAction::physics); // Can only be this due to wantTick
	//_rotation *= glm::angleAxis(0.0005f, glm::vec3(0.15, 0.8, 0.2) * _rotation);
	_dirty = true;
	_drawObject->modelMatrix = getMatrix();
}


void TransformComponent::serialize(nlohmann::json& json) const {
	json = {
		{"position", {_position.x, _position.y, _position.z}},
		{"scale", {_scale.x, _scale.y, _scale.z}},
		{"rotation", {_rotation.x, _rotation.y, _rotation.z, _rotation.w}}
	};
}

void TransformComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};

	auto& scale = json["scale"];
	_scale = glm::vec3{scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>()};

	auto& rotation = json["rotation"];
	_rotation = glm::quat{rotation[0].get<float>(), rotation[1].get<float>(), rotation[2].get<float>(), rotation[3].get<float>()};
}

void TransformComponent::registerUI() {
	_dirty |= ImGui::DragFloat3("Position", glm::value_ptr(_position), 0.01f);
	_dirty |= ImGui::DragFloat3("Scale", glm::value_ptr(_scale), 0.01f);
	_dirty |= ImGui::DragFloat4("Rotation", glm::value_ptr(_rotation), 0.01f);
}

void TransformComponent::setPosition(const glm::vec3& position) {
	_dirty |= _position != position;
	_position = position;
}

void TransformComponent::setScale(const glm::vec3& scale) {
	_dirty |= _scale != scale;
	_scale = scale;
}

void TransformComponent::setRotation(const glm::quat& rotation) {
	_dirty |= _rotation != rotation;
	_rotation = rotation;
}

void TransformComponent::setDirection(const glm::vec3& direction, glm::vec3 up) {
	if (direction == up)
		up.x += 0.0001f;

	static const glm::vec3 O = {0, 0, 0};
	glm::mat3 m = glm::lookAt(O, direction, up);
	setRotation(glm::quat_cast(m));
}

void TransformComponent::_recalculateMatrix() {
	auto p = _getParentComponent();
	glm::mat4 parent = p ? p->getMatrix() : glm::mat4(1);
	_dirty = false;

	_matrix = parent * (glm::translate(_position) * glm::mat4_cast(_rotation) * glm::scale(_scale));
}

TransformComponent* TransformComponent::_getParentComponent() {
	return entity->getParent()->getComponent<TransformComponent>();
}
