#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	enum PrimitiveType : int {quad = 0};
	class HYDRA_API MeshComponent final : public IComponent {
	public:
		MeshComponent(IEntity* entity, const std::string& meshFile);
		MeshComponent(IEntity* entity, const PrimitiveType& type);
		~MeshComponent() final;

		void tick(TickAction action) final;
		inline TickAction wantTick() const final { return TickAction::render; }

		inline const std::string type() const final { return "MeshComponent"; }

		msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const final;
		void registerUI() final;

	private:
		std::string _meshFile;
		Hydra::Renderer::DrawObject* _drawObject;
		std::shared_ptr<Hydra::Renderer::IMesh> _mesh;
	};
};
