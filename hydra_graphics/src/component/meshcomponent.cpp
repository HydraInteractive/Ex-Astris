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

MeshComponent::MeshComponent(IEntity* entity, const std::string& meshFile) : IComponent(entity), _meshFile(meshFile), _drawObject(entity->getDrawObject()) {
	_mesh = Hydra::IEngine::getInstance()->getMeshLoader()->getMesh(meshFile);
	_drawObject->refCounter++;
}

MeshComponent::MeshComponent(IEntity* entity, const PrimitiveType& type) : IComponent(entity), _drawObject(entity->getDrawObject()) {
	_mesh = Hydra::IEngine::getInstance()->getMeshLoader()->getQuad();
	_drawObject->refCounter++;
}

MeshComponent::~MeshComponent() {
	_drawObject->mesh = nullptr;
	_drawObject->refCounter--;
}

void MeshComponent::tick(TickAction action) {
	// assert(action == TickAction::render); // Can only be this due to wantTick
	_drawObject->mesh = _mesh.get();
}

msgpack::packer<msgpack::sbuffer>& MeshComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	o.pack_map(1);
	o.pack("meshFile");
	o.pack(_meshFile);

	return o;
}

void MeshComponent::registerUI() {
	ImGui::Checkbox("Disable", &_drawObject->disable);
	ImGui::InputText("Mesh file", (char*)_meshFile.c_str(), _meshFile.length(), ImGuiInputTextFlags_ReadOnly);
}
