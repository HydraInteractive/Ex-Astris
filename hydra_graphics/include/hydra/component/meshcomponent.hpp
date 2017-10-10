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
	struct HYDRA_API MeshComponent final : public IComponent<MeshComponent, ComponentBits::Mesh> {
		std::string meshFile;
		Hydra::Renderer::DrawObject* drawObject;
		std::shared_ptr<Hydra::Renderer::IMesh> mesh;

		~MeshComponent() final;

		void loadMesh(const std::string meshFile);

		inline const std::string type() const final { return "MeshComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
