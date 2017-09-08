/**
 * A mesh component that specifies that the entity can be rendered.
 * It contains a DrawObject instance from the renderer.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	class HYDRA_API MeshComponent final : public IComponent {
	public:
		MeshComponent(IEntity* entity);
		MeshComponent(IEntity* entity, const std::string& meshFile);
		~MeshComponent() final;

		void tick(TickAction action) final;
		inline TickAction wantTick() const final { return TickAction::render; }

		inline const std::string type() const final { return "MeshComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		std::string _meshFile;
		Hydra::Renderer::DrawObject* _drawObject;
		std::shared_ptr<Hydra::Renderer::IMesh> _mesh;
	};
};
