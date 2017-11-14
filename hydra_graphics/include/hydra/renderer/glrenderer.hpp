/**
 * An implementation for the IRender, IMesh, ITexture and IFramebuffer, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/renderer/renderer.hpp>
#include <hydra/view/view.hpp>

#include <memory>
#include <vector>
#include <cstdint>

namespace Hydra::Renderer {
	namespace GLRenderer {
		HYDRA_GRAPHICS_API std::unique_ptr<IRenderer> create(Hydra::View::IView& view);
	};

	namespace GLMesh {
		HYDRA_GRAPHICS_API std::unique_ptr<IMesh> create(const std::string& file, IRenderer* renderer);
		HYDRA_GRAPHICS_API std::unique_ptr<IMesh> createQuad(IRenderer* renderer);
		HYDRA_GRAPHICS_API std::unique_ptr<IMesh> createFullscreenQuad();
		HYDRA_GRAPHICS_API std::unique_ptr<IMesh> createCube();
	};

	namespace GLTexture {
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createFromID(uint32_t id, glm::ivec2 size, TextureType format = TextureType::u8RGBA, size_t samples = 0, bool own = true);
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createFromFile(const std::string& file);
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createEmpty(uint32_t width, uint32_t height, TextureType format = TextureType::u8RGBA, size_t samples = 0);
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createFromData(uint32_t width, uint32_t height, TextureType format, void* data);
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createFromDataExt(const char* ext, void* data, uint32_t size);
		HYDRA_GRAPHICS_API std::shared_ptr<ITexture> createDataTexture(uint32_t width, uint32_t height, TextureType format);
	};

	namespace GLFramebuffer {
		HYDRA_GRAPHICS_API std::shared_ptr<IFramebuffer> create(glm::ivec2 size, size_t samples);
	};
}
