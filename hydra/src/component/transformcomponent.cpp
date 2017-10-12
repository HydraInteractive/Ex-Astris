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

TransformComponent::~TransformComponent() {}

void TransformComponent::serialize(nlohmann::json& json) const {
	json = {
		{"position", {position.x, position.y, position.z}},
		{"scale", {scale.x, scale.y, scale.z}},
		{"rotation", {rotation.w, rotation.x, rotation.y, rotation.z}},
		{"ignoreParent", ignoreParent}
	};
}

void TransformComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	this->position = glm::vec3{pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};

	auto& scale = json["scale"];
	this->scale = glm::vec3{scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>()};

	auto& rotation = json["rotation"];
	this->rotation = glm::quat{rotation[0].get<float>(), rotation[1].get<float>(), rotation[2].get<float>(), rotation[3].get<float>()};

	this->ignoreParent = json["ignoreParent"].get<bool>();
}

void TransformComponent::registerUI() {
	dirty |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
	dirty |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f);
	dirty |= ImGui::DragFloat4("Rotation", glm::value_ptr(rotation), 0.01f);
	dirty |= ImGui::Checkbox("Ignore parent", &ignoreParent);
}

void TransformComponent::_recalculateMatrix() {
	auto p = _getParentComponent();
	glm::mat4 parent = p ? p->getMatrix() : glm::mat4(1);
	dirty = false;

	_matrix = parent * (glm::translate(position) * glm::mat4_cast(rotation) * glm::scale(scale));
}

std::shared_ptr<Hydra::Component::TransformComponent> TransformComponent::_getParentComponent() {
	auto parent = Hydra::World::World::getEntity(entityID)->parent;
	return (ignoreParent || !parent) ? std::shared_ptr<TransformComponent>() : Hydra::World::World::getEntity(parent)->getComponent<TransformComponent>();
}
