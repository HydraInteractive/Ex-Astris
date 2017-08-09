#pragma once

#include <hydra/renderer/renderer.hpp>
#include <hydra/view/view.hpp>

#include <memory>
#include <vector>
#include <cstdint>

namespace Hydra::Renderer {
	namespace GLRenderer {
		std::unique_ptr<IRenderer> create(Hydra::View::IView& view);
	};

	namespace GLMesh {
		std::unique_ptr<IMesh> create(const std::string& file, IRenderer* renderer);
	};

	namespace GLTexture {
		std::shared_ptr<ITexture> createFromFile(const std::string& file);
		std::shared_ptr<ITexture> createFromData(uint32_t width, uint32_t height, void* data);
		std::shared_ptr<ITexture> createFromDataExt(const char* ext, void* data, uint32_t size);
	};
}
