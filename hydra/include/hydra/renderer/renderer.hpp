#pragma once

#include <glm/glm.hpp>
#include <hydra/view/view.hpp>

namespace Hydra::Renderer {
	class IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void bind(Hydra::View::IRenderTarget* renderTarget) = 0;
		virtual void clear(glm::vec4 clearColor) = 0;
	};

	inline IRenderer::~IRenderer() {}
}
