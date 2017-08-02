#include <hydra/component/meshcomponent.hpp>

#include <hydra/renderer/glrenderer.hpp>

#include <hydra/engine.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

extern Hydra::IEngine* engineInstance;

MeshComponent::MeshComponent(std::weak_ptr<IEntity> entity, const std::vector<Hydra::Renderer::Vertex>& vertices, const std::vector<uint32_t>& indices) : IComponent(entity), _meshFile("") {
	_mesh = Hydra::Renderer::GLMesh::create(vertices, indices);
}
MeshComponent::MeshComponent(std::weak_ptr<IEntity> entity, const std::string& meshFile) : IComponent(entity), _meshFile(meshFile) {
}
MeshComponent::~MeshComponent() {}

void MeshComponent::tick(TickAction action) {
	if (action == TickAction::render)
		engineInstance->getRenderer()->render(*_mesh, 1);
}

msgpack::packer<msgpack::sbuffer>& MeshComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	o.pack_map(1);
	o.pack("meshFile");
	o.pack(_meshFile);

	return o;
}
