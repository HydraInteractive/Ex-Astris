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

MeshComponent::~MeshComponent() {
	_drawObject->mesh = nullptr;
	_drawObject->refCounter--;
}

void MeshComponent::tick(TickAction action) {
	if (action == TickAction::render)
		_drawObject->mesh = _mesh.get();
}

msgpack::packer<msgpack::sbuffer>& MeshComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	o.pack_map(1);
	o.pack("meshFile");
	o.pack(_meshFile);

	return o;
}

void MeshComponent::registerUI() {
	ImGui::InputText("Mesh file", (char*)_meshFile.c_str(), _meshFile.length(), ImGuiInputTextFlags_ReadOnly);
}
