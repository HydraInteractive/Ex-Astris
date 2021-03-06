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
	if (!Hydra::IEngine::getInstance()->getState()->getMeshLoader())
		return;
	drawObject = Hydra::World::World::getEntity(entityID)->addComponent<DrawObjectComponent>();
	mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getMesh(meshFile);
	auto newMesh = drawObject->drawObject->mesh = mesh.get();

	if (meshFile == "PARTICLEQUAD" || meshFile == "TEXTQUAD" || meshFile == "QUAD")
		drawObject->drawObject->disable = true;

	if (meshFile.find("Wall") != std::string::npos || meshFile.find("Tunnel") != std::string::npos
		 || meshFile.find("Roof") != std::string::npos || meshFile.find("Floor_v") != std::string::npos
		|| meshFile.find("HangingCable") != std::string::npos || meshFile.find("Monitor") != std::string::npos
		|| meshFile.find("BookShelf") != std::string::npos || meshFile.find("Pillar") != std::string::npos || meshFile.find("Locker") != std::string::npos
		|| meshFile.find("WaterContainer") != std::string::npos || meshFile.find("EnglishWordForHyllla") != std::string::npos
		|| meshFile.find("Fridge") != std::string::npos || meshFile.find("Door") != std::string::npos){
		drawObject->drawObject->hasShadow = false;
	}
}

void MeshComponent::serialize(nlohmann::json& json) const {
	json["meshFile"] = meshFile;
	json["currentFrame"] = currentFrame;
	json["animationIndex"] = animationIndex;
	json["animationCounter"] = animationCounter;
}

void MeshComponent::deserialize(nlohmann::json& json) {
	loadMesh(json["meshFile"].get<std::string>());
	currentFrame = json.value<int>("currentFrame", 1);
	animationIndex = json.value<int>("animationIndex", 0);
	animationCounter = json.value<float>("animationCounter", 0);
}

void MeshComponent::registerUI() {
	ImGui::InputText("Mesh file", (char*)meshFile.c_str(), meshFile.length(), ImGuiInputTextFlags_ReadOnly);
	ImGui::DragInt("CurrentFrame", &currentFrame);
	ImGui::DragInt("AnimationIndex", &animationIndex);
	ImGui::DragFloat("AnimationCounter", &animationCounter);
}
