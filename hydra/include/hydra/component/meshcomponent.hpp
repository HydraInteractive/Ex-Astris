#pragma once

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class MeshComponent : public IComponent {
	public:
		MeshComponent(std::weak_ptr<IEntity> entity, const std::vector<Hydra::Renderer::Vertex>& vertices, const std::vector<uint32_t>& indices);
		MeshComponent(std::weak_ptr<IEntity> entity, const std::string& meshFile);
		~MeshComponent() final;

		void tick(TickAction action) final;

		inline const std::string type() final { return "MeshComponent"; }

		msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const final;

	private:
		std::string _meshFile;
		std::unique_ptr<Hydra::Renderer::IMesh> _mesh;
	};
};
