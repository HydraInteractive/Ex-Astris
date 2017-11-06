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

#include <hydra/component/cameracomponent.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <imgui/imgui.h>

#include <hydra/engine.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

TransformComponent* TransformComponent::_currentlyEditing = nullptr;

TransformComponent::~TransformComponent() {
	if (_currentlyEditing == this)
		_currentlyEditing = nullptr;
}

void TransformComponent::serialize(nlohmann::json& json) const {
	json = {
		{"position", {position.x, position.y, position.z}},
		{"scale", {scale.x, scale.y, scale.z}},
		{"rotation", {rotation.x, rotation.y, rotation.z, rotation.w}},
		{"ignoreParent", ignoreParent}
	};
}

void TransformComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	position = glm::vec3{pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()};

	auto& s = json["scale"];
	scale = glm::vec3{s[0].get<float>(), s[1].get<float>(), s[2].get<float>()};

	auto& rot = json["rotation"];
	rotation = glm::quat{rot[3].get<float>(), rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>()};

	ignoreParent = json["ignoreParent"].get<bool>();
}

void TransformComponent::registerUI() {
	dirty |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
	dirty |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f);
	dirty |= ImGui::DragFloat4("Rotation", glm::value_ptr(rotation), 0.01f);
	dirty |= ImGui::Checkbox("Ignore parent", &ignoreParent);

	if (ImGui::Button("Enable Guizmo"))
		_currentlyEditing = this;
}

void TransformComponent::showGuizmo() {
	auto& this_ = _currentlyEditing;
	if (!this_)
		return;

	auto p = this_->_getParentComponent();
	glm::mat4 parent = p ? glm::inverseTranspose(p->getMatrix()) : glm::mat4(1);
	//this_->_recalculateMatrix();
	glm::mat4 matrix = this_->getMatrix();//_matrix;//glm::translate(this_->position) * glm::mat4_cast(glm::normalize(this_->rotation)) * glm::scale(this_->scale);

	static ImGuizmo::OPERATION currentOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE currentMode(ImGuizmo::WORLD);

	ImGui::Begin("Guizmo tools");
	ImGui::Text("Currently editing: %s", Hydra::World::World::getEntity(this_->entityID)->name.c_str());

	if (ImGui::IsKeyPressed(SDLK_z))
		currentOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(SDLK_x))
		currentOperation = ImGuizmo::ROTATE;
	/*if (ImGui::IsKeyPressed(SDLK_c))
		currentOperation = ImGuizmo::SCALE;*/

	ImGui::Text("<Z> Translate, <X> Rotate"); // , <C> Scale

	if (ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
		currentOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
		currentOperation = ImGuizmo::ROTATE;
	/*ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
		currentOperation = ImGuizmo::SCALE;*/

	if (currentOperation != ImGuizmo::SCALE) {
		if (ImGui::RadioButton("Local", currentMode == ImGuizmo::LOCAL))
			currentMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", currentMode == ImGuizmo::WORLD))
			currentMode = ImGuizmo::WORLD;
	}

	static bool useSnap(false);
	if (ImGui::IsKeyPressed(83))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();

	glm::vec3* snap;
	switch (currentOperation) {
	case ImGuizmo::TRANSLATE:
		static glm::vec3 snapTranslation = glm::vec3(0.1);
		snap = &snapTranslation;
		ImGui::InputFloat3("Snap", glm::value_ptr(snapTranslation));
		break;
	case ImGuizmo::ROTATE:
		static glm::vec3 snapRotation = glm::vec3(0.1);
		snap = &snapRotation;
		ImGui::InputFloat("Angle Snap", glm::value_ptr(snapRotation));
		break;
	case ImGuizmo::SCALE:
		/*static glm::vec3 snapScale = glm::vec3(0.1);
			snap = &snapScale;
			ImGui::InputFloat("Scale Snap", glm::value_ptr(snapScale));*/
		break;
	}
	ImGui::End();

	//TODO: Make argument
	auto* cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());

	glm::mat4 deltaMatrix;
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(glm::value_ptr(cc->getViewMatrix()), glm::value_ptr(cc->getProjectionMatrix()), currentOperation, currentMode, glm::value_ptr(matrix), glm::value_ptr(deltaMatrix), useSnap ? glm::value_ptr(*snap) : nullptr);
	this_->dirty = true;
	matrix *= parent;

	static bool didUse = false;
	if (ImGuizmo::IsUsing())/*
		didUse = true;
	else if (didUse)*/ {
		didUse = false;
		switch(currentOperation) {
		case ImGuizmo::TRANSLATE:
			this_->position += glm::vec3(deltaMatrix[3]);
			break;
		case ImGuizmo::ROTATE:
			this_->rotation = glm::toQuat(deltaMatrix) * this_->rotation;
			break;
		case ImGuizmo::SCALE:
				/*//this_->scale += glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]); //glm::scale(matrix);
			glm::vec3 translation;
			glm::vec3 rotation;
			glm::vec3 scale;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
			this_->scale = scale;
			// this_->scale *= glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]);*/
			break;
		}
	}
}

void TransformComponent::_recalculateMatrix() {
	auto p = _getParentComponent();
	if (!dirty && !(p && (p->dirty || p->updateCounter != parentUpdateCounter)))
		return;
	updateCounter++;
	glm::mat4 parent = p ? p->getMatrix() : glm::mat4(1);
	if (p)
		parentUpdateCounter = p->updateCounter;
	dirty = false;

	_matrix = parent * (glm::translate(position) * glm::mat4_cast(glm::normalize(rotation)) * glm::scale(scale));
}

std::shared_ptr<Hydra::Component::TransformComponent> TransformComponent::_getParentComponent() {
	auto parent = Hydra::World::World::getEntity(entityID)->parent;
	return (ignoreParent || !parent) ? std::shared_ptr<TransformComponent>() : Hydra::World::World::getEntity(parent)->getComponent<TransformComponent>();
}
