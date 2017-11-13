#include <barcode/renderingutils.hpp>

#include <hydra/renderer/gltexture.hpp>
#include <hydra/renderer/glshader.hpp>

#include <glm/glm.hpp>

namespace Barcode {

	BlurUtil::BlurUtil(const glm::ivec2& size, Hydra::Renderer::TextureType type) {
		_batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/blur.vert");
		_batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/blur.frag");

		_batch.pipeline = Hydra::Renderer::GLPipeline::create();
		_batch.pipeline->attachStage(*batch.vertexShader);
		_batch.pipeline->attachStage(*batch.fragmentShader);
		_batch.pipeline->finalize();
		_batch.pipeline->setValue(1, 1);

		_batch.output = nullptr;

		_batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
		_batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color;
		_batch.batch.renderTarget = batch.output.get();
		_batch.batch.pipeline = batch.pipeline.get();

		_fbo0 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
		_fbo0->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();
		_fbo1 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
		_fbo1->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();
	}

	BlurUtil::~BlurUtil() { }

	std::shared_ptr<Hydra::Renderer::IFramebuffer> BlurUtil::blur(std::shared_ptr<Hydra::Renderer::ITexture>& texture, size_t nrOfTimes, const glm::vec2& size) {
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
				(*_fbo0)[0]->bind(1);
			} else {
				_batch.batch.renderTarget = _fbo0.get();
				(*_fbo1)[0]->bind(1);
			}
			_batch.pipeline->setValue(2, horizontal);
			_engine->getRenderer()->postProcessing(_batch.batch);
			horizontal = !horizontal;
		}

		return _fbo0;
	}
}
