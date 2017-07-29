#pragma once

#include <hydra/renderer/renderer.hpp>
#include <hydra/view/view.hpp>

#include <memory>
#include <vector>
#include <cstdint>

namespace Hydra::Renderer {
	struct GLRenderer final {
		GLRenderer() = delete;

		static std::unique_ptr<IRenderer> create(Hydra::View::IView& view);
	};

	struct GLMesh final {
		GLMesh() = delete;

		static std::unique_ptr<IMesh> create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		static std::unique_ptr<IMesh> createFromFile(const std::string& file);
	};
}
