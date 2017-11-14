#pragma once

#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>

#include <glm/glm.hpp>

#include <type_traits>

namespace Barcode {
	struct RenderBatchBase {
		std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
		std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
		std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

		std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
	};

	template <class T, class Enable = void>
	struct RenderBatch;
	template <class T>
	struct RenderBatch<T, typename std::enable_if_t<std::is_base_of_v<Hydra::Renderer::Batch, T>>> final : public RenderBatchBase {
		T batch;

		inline RenderBatch() {}

		RenderBatch(const std::string& vertex, const std::string& geometry, const std::string& fragment, const glm::ivec2& size) {
			_setupShader(vertex, geometry, fragment);
			output = Hydra::Renderer::GLFramebuffer::create(size, 0);
			_setupBatch(output.get());
		}

		RenderBatch(const std::string& vertex, const std::string& geometry, const std::string& fragment, Hydra::Renderer::IRenderTarget* rt) {
			_setupShader(vertex, geometry, fragment);
			_setupBatch(rt);
		}

		template <class T2>
		RenderBatch(const std::string& vertex, const std::string& geometry, const std::string& fragment, RenderBatch<T2>& rb) {
			_setupShader(vertex, geometry, fragment);
			_setupBatch(rb.batch.renderTarget);
		}

	private:
		void _setupShader(const std::string& vertex, const std::string& geometry, const std::string& fragment) {
			pipeline = Hydra::Renderer::GLPipeline::create();
			if (vertex.size()) {
				vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, vertex);
				pipeline->attachStage(*vertexShader);
			}
			if (geometry.size()) {
				geometryShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::geometry, geometry);
				pipeline->attachStage(*geometryShader);
			}
			if (fragment.size()) {
				fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, fragment);
				pipeline->attachStage(*fragmentShader);
			}
			pipeline->finalize();
			batch.pipeline = pipeline.get();
		}

		void _setupBatch(Hydra::Renderer::IRenderTarget* rt) {
			batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.renderTarget = rt;
		}
	};

	class BlurUtil final {
	public:
		BlurUtil(Hydra::Renderer::IRenderer* renderer, const glm::ivec2& size, Hydra::Renderer::TextureType type);
		~BlurUtil();

		void blur(std::shared_ptr<Hydra::Renderer::ITexture>& texture, size_t nrOfTimes, const glm::vec2& size);

		std::shared_ptr<Hydra::Renderer::ITexture>& getOutput();

	private:
		Hydra::Renderer::IRenderer* _renderer;
		RenderBatch<Hydra::Renderer::Batch> _batch;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _fbo0;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _fbo1;
	};
};
