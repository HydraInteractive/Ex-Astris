#include <barcode/renderingutils.hpp>

namespace Barcode {
	BlurUtil::BlurUtil(Hydra::Renderer::IRenderer* renderer, const glm::ivec2& size, Hydra::Renderer::TextureType type) : _renderer(renderer) {
		_batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/blur.vert");
		_batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/blur.frag");

		_batch.pipeline = Hydra::Renderer::GLPipeline::create();
		_batch.pipeline->attachStage(*_batch.vertexShader);
		_batch.pipeline->attachStage(*_batch.fragmentShader);
		_batch.pipeline->finalize();
		_batch.pipeline->setValue(1, 1);

		_batch.output = nullptr;

		_batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
		_batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color;
		_batch.batch.renderTarget = _batch.output.get();
		_batch.batch.pipeline = _batch.pipeline.get();

		_fbo0 = Hydra::Renderer::GLFramebuffer::create(size, 0);
		_fbo0->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();
		_fbo1 = Hydra::Renderer::GLFramebuffer::create(size, 0);
		_fbo1->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();
	}

	BlurUtil::~BlurUtil() { }

	void BlurUtil::blur(std::shared_ptr<Hydra::Renderer::ITexture>& texture, size_t nrOfTimes, const glm::vec2& size) {
		bool horizontal = true;
		bool firstPass = true;
		_fbo0->resize(size);
		_fbo1->resize(size);

		for (size_t i = 0; i < nrOfTimes * 2; i++) {
			if (firstPass) {
				_batch.batch.renderTarget = _fbo1.get();
				texture->bind(1);
				firstPass = false;
			} else if (horizontal) {
				_batch.batch.renderTarget = _fbo1.get();
				_fbo0->operator[](0)->bind(1);
			} else {
				_batch.batch.renderTarget = _fbo0.get();
				_fbo1->operator[](0)->bind(1);
			}
			_batch.pipeline->setValue(2, horizontal);
			_renderer->postProcessing(_batch.batch);
			horizontal = !horizontal;
		}
	}

	std::shared_ptr<Hydra::Renderer::ITexture>& BlurUtil::getOutput() {
		return _fbo1->operator[](0);
	}
}
