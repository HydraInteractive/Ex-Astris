// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A mesh component that specifies that the entity can be rendered.
 * It contains a DrawObject instance from the renderer.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/component/meshcomponent.hpp>

#include <imgui/imgui.h>
#include <hydra/renderer/glrenderer.hpp>

#include <hydra/engine.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

MeshComponent::~MeshComponent() {}

void MeshComponent::loadMesh(const std::string meshFile) {
	this->meshFile = meshFile;
	drawObject = Hydra::World::World::getEntity(entityID)->addComponent<DrawObjectComponent>();
	mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getMesh(meshFile);
	drawObject->drawObject->mesh = mesh.get();

	if (meshFile == "QUAD")
		drawObject->drawObject->disable = true;
}

void MeshComponent::serialize(nlohmann::json& json) const {
	json["meshFile"] = meshFile;
	json["currentFrame"] = currentFrame;
	json["animationCounter"] = animationCounter;
}

void MeshComponent::deserialize(nlohmann::json& json) {
	loadMesh(json["meshFile"].get<std::string>());
	currentFrame = json["currentFrame"].get<int>();
	animationCounter = json["animationCounter"].get<float>();
}

void MeshComponent::registerUI() {
	ImGui::InputText("Mesh file", (char*)meshFile.c_str(), meshFile.length(), ImGuiInputTextFlags_ReadOnly);
}
