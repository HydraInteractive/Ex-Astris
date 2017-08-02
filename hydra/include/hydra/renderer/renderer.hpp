#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <memory>

#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	enum /*class*/ VertexLocation : uint32_t {
		position = 0,
		normal = 1,
		color = 2,
		uv = 3,
		tangent = 4
	};

	struct Vertex final {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 tangent;
	};

	class ITexture {
	public:
		virtual ~ITexture() = 0;

		virtual uint32_t getID() = 0;
	};
	inline ITexture::~ITexture() {}

	class IRenderTarget : public ITexture {
	public:
		virtual ~IRenderTarget() = 0;

		virtual void finalize() = 0;
	};
	inline IRenderTarget::~IRenderTarget() {}

	struct Material final {
		std::shared_ptr<ITexture> diffuse;
		std::shared_ptr<ITexture> normal;
	};

	class IMesh {
	public:
		virtual ~IMesh() = 0;

		virtual Material& getMaterial() = 0;
	};
	inline IMesh::~IMesh() {}

	enum class RenderOrder {
		frontToBack = 0,
		backToFront = 1
	};

	class IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void setRenderOrder(RenderOrder renderOrder) = 0;

		virtual void bind(IRenderTarget& renderTarget) = 0;
		virtual void clear(glm::vec4 clearColor) = 0;

		virtual void use(IPipeline& pipeline) = 0;

		virtual void render(IMesh& mesh, size_t instances) = 0;

		/// Render all created batches
		virtual void flush() = 0;
	};
	inline IRenderer::~IRenderer() {}
}
