/**
 * Everything related to rendering, as an interface.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <map>
#include <vector>

#include <hydra/renderer/shader.hpp>

//Influences is how much a joint transform affect the vertex. Controllers
//are which joints that influence the vertex. Each vertex has 4 controllers
//and the summary of all influences should be 1

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
		_modelMatrix3 = 8,
		influences = 9,
		controllers = 10,
		textureOffset1 = 11,
		textureOffset2 = 12,
		textureCoordInfo = 13,
		charRect = 14,
		charPos = 15
	};

	struct CharRenderInfo {
		// charRect.xy = [startX, startY] in texture
		// charRect.zw = [width, height] of char
		// charPos     = [xPos, yPos, zPos]
		glm::vec4 charRect;
		glm::vec3 charPos;
	};

	struct HYDRA_BASE_API Vertex final {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 tangent;
		glm::vec4 influences;
		glm::ivec4 controllers;
	};

	class HYDRA_BASE_API ITexture {
	public:
		virtual ~ITexture() = 0;

		virtual void resize(glm::ivec2 size) = 0;
		virtual void bind(size_t position) = 0;

		virtual glm::ivec2 getSize() = 0;
		virtual size_t getSamples() = 0;
		virtual uint32_t getID() const = 0;
		virtual void setRepeat() = 0;
		virtual void setClamp() = 0;

		virtual void setData(const glm::ivec2& offset, const glm::ivec2& size, const void* data) = 0;
	};
	inline ITexture::~ITexture() {}

	class HYDRA_BASE_API IRenderTarget : public ITexture {
	public:
		virtual ~IRenderTarget() = 0;
		inline void setRepeat() final{}
		inline void setClamp() final{}
	};
	inline IRenderTarget::~IRenderTarget() {}

	enum class HYDRA_BASE_API TextureType {
		u8R = 0,
		u8RG,
		u8RGB,
		u8RGBA,

		f16R,
		f16RG,
		f16RGB,
		f16RGBA,

		f32R,
		f32RG,
		f32RGB,
		f32RGBA,

		// These needs to be last due to some code in the framebuffer
		f16Depth,
		f24Depth,
		f32Depth
	};

	class HYDRA_BASE_API IFramebuffer : public IRenderTarget {
	public:
		virtual ~IFramebuffer() = 0;

		virtual IFramebuffer& addTexture(size_t id, TextureType type) = 0;

		virtual void finalize() = 0;

		inline void setData(const glm::ivec2&, const glm::ivec2&, const void*) { }

		virtual std::shared_ptr<ITexture> getDepth() = 0;

		virtual std::shared_ptr<ITexture>& operator[](size_t idx) = 0;
	};
	inline IFramebuffer::~IFramebuffer() {}

	struct HYDRA_BASE_API Material final {
		std::shared_ptr<ITexture> diffuse;
		std::shared_ptr<ITexture> normal;
		std::shared_ptr<ITexture> glow;
		std::shared_ptr<ITexture> specular;
	};

	class HYDRA_BASE_API IMesh {
	public:
		virtual ~IMesh() = 0;

		virtual Material& getMaterial() = 0;
		virtual bool hasAnimation() = 0;
		virtual glm::mat4 getTransformationMatrices(int currAnimIdx, int joint, int currentFrame) = 0;
		virtual int getNrOfJoints(int currAnimIdx) = 0;
		virtual int getCurrentKeyframe() = 0;
		virtual int getMaxFramesForAnimation(int currAnimIdx) = 0;
		virtual int getCurrentAnimationIndex() = 0;
		virtual float& getAnimationCounter() = 0;
		virtual void setCurrentKeyframe(int frame) = 0;
		virtual void setAnimationIndex(int index) = 0;
		virtual uint32_t getID() const = 0;
		virtual size_t getIndicesCount() const = 0;
	};
	inline IMesh::~IMesh() {}

	// Components updates this object
	// Renderer uses these to know what to render and where
	// TODO: Maybe add DrawObject for transparent stuff or just field
	struct HYDRA_BASE_API DrawObject final {
		std::vector<glm::mat4> transfomationMatrices;
		size_t refCounter = 0;
		bool hasShadow = true;
		bool disable = false; // Temporarily disable object
		IMesh* mesh = nullptr; // & Material // TODO: Change to something else than IMesh?
		glm::mat4 modelMatrix = glm::mat4(1);
	};

	struct HYDRA_BASE_API Camera final {
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	enum class HYDRA_BASE_API ClearFlags {
		none = 0,
		color = 1 << 0,
		depth = 1 << 1
	};
	inline ClearFlags operator| (ClearFlags a, ClearFlags b) { return static_cast<ClearFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
	inline ClearFlags operator& (ClearFlags a, ClearFlags b) { return static_cast<ClearFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	struct HYDRA_BASE_API Batch {
		glm::vec4 clearColor;
		ClearFlags clearFlags;
		IRenderTarget* renderTarget;
		IPipeline* pipeline;
		std::map<IMesh*, std::vector<glm::mat4 /* Model matrix */>> objects;
	};

	struct HYDRA_BASE_API AnimationBatch : public Batch {
		std::map<IMesh*, std::vector<int>> currentFrames;
		std::map<IMesh*, std::vector<int>> currAnimIndices;
	};

	struct HYDRA_BASE_API ParticleBatch : public Batch {
		std::vector<glm::vec2> textureInfo;
	};


	struct HYDRA_BASE_API TextBatch : public Batch{
		std::vector<CharRenderInfo> textInfo;
		std::vector<size_t> textSizes;
		std::vector<float> lifeFade;
		std::vector<glm::vec3> colors;
	};

	class HYDRA_BASE_API IRenderer {
	public:
		virtual ~IRenderer() = 0;

		virtual void render(Batch& batch) = 0;
		virtual void renderAnimation(AnimationBatch& batch) = 0;
		virtual void render(ParticleBatch& batch) = 0;
		virtual void renderShadows(Batch& batch) = 0;
		virtual void renderShadows(AnimationBatch& batch) = 0;
		virtual void renderText(TextBatch& batch) = 0;
		// Note: this will ignore batch.objects
		virtual void postProcessing(Batch& batch) = 0;
		virtual void renderHitboxes(Batch& batch) = 0;

		virtual DrawObject* aquireDrawObject() = 0;

		virtual void showGuizmo() = 0;

		virtual const std::vector<std::unique_ptr<DrawObject>>& activeDrawObjects() = 0;

		virtual void cleanup() = 0;
		virtual void clear(Batch& batch) = 0;

		virtual void* getModelMatrixBuffer() = 0;
		virtual void* getParticleExtraBuffer() = 0;
		virtual void* getTextExtraBuffer() = 0;
	};
	inline IRenderer::~IRenderer() {}
}
