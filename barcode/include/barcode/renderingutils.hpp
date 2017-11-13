#pragma once

#include <hydra/renderer/renderer.hpp>

namespace Barcode {
	struct RenderBatch final {
		std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
		std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
		std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

		std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
		Hydra::Renderer::Batch batch;
	};

	struct AnimationRenderBatch final {
		std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
		std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
		std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

		std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
		Hydra::Renderer::AnimationBatch batch;
	};

	struct ParticleRenderBatch final {
		std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
		std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
		std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

		std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
		Hydra::Renderer::ParticleBatch batch;
	};

	class BlurUtil final {
	public:
		BlurUtil(const glm::ivec2& size, Hydra::Renderer::TextureType type);
		~BlurUtil();

		Hydra::Renderer::IFramebuffer* blur(std::shared_ptr<Hydra::Renderer::ITexture>& texture, size_t nrOfTimes, const glm::vec2& size);

	private:
		RenderBatch _batch;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _fbo0;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _fbo1;
	};
};
