#pragma once

#include <glm/glm.hpp>
#include <cstdint>

#include <hydra/view/view.hpp>
#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	enum /*class*/ VertexLocation : uint32_t {
		position = 0,
		normal = 1,
		color = 2,
		uv = 3,
		tangent = 4
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 tangent;
	};

	class IMesh {
	public:
		virtual ~IMesh() = 0;
	};
	inline IMesh::~IMesh() {}

	class IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void bind(Hydra::View::IRenderTarget& renderTarget) = 0;
		virtual void clear(glm::vec4 clearColor) = 0;

		virtual void use(IPipeline& pipeline) = 0;

		virtual void render(IMesh& mesh, size_t instances) = 0;
	};
	inline IRenderer::~IRenderer() {}
}
