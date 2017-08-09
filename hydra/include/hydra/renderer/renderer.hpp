#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <map>
#include <vector>

#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	enum /*class*/ VertexLocation : uint32_t {
		position = 0,
		normal = 1,
		color = 2,
		uv = 3,
		tangent = 4,
		modelMatrix = 5,
		_modelMatrix1 = 6,
		_modelMatrix2 = 7,
		_modelMatrix3 = 8
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

		virtual glm::ivec2 getSize() = 0;
		virtual uint32_t getID() const = 0;
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

	// Components updates this object
	// Renderer uses these to know what to render and where
	// TODO: Maybe add DrawObject for transparent stuff or just field
	struct DrawObject final {
		int refCounter = 0;
		// bool disable; // TODO: ?
		IMesh* mesh = nullptr; // & Material // TODO: Change to something else than IMesh?
		glm::mat4 modelMatrix;
	};

	struct Camera final {
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	enum class ClearFlags {
		color = 1 << 0,
		depth = 1 << 1,
	};
	inline ClearFlags operator| (ClearFlags a, ClearFlags b) { return static_cast<ClearFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
	inline ClearFlags operator& (ClearFlags a, ClearFlags b) { return static_cast<ClearFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	struct Batch {
		glm::vec4 clearColor;
		ClearFlags clearFlags;
		IRenderTarget* renderTarget;
		IPipeline* pipeline;
		// Camera* camera;
		std::map<IMesh*, std::vector<glm::mat4 /* Model matrix */>> objects;
	};

	class IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void render(Batch& batch) = 0;

		virtual DrawObject* aquireDrawObject() = 0;

		virtual std::vector<DrawObject*> activeDrawObjects() = 0;

		virtual void cleanup() = 0;
	};
	inline IRenderer::~IRenderer() {}
}
