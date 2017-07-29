#pragma once

#include <glm/glm.hpp>
#include <hydra/view/view.hpp>
#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	class IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void bind(Hydra::View::IRenderTarget& renderTarget) = 0;
		virtual void clear(glm::vec4 clearColor) = 0;

		virtual void use(IPipeline& pipeline) = 0;
	};
	inline IRenderer::~IRenderer() {}

	class IMesh {
	public:
		virtual ~IMesh() = 0;
	};
	inline IMesh::~IMesh() {}
}
