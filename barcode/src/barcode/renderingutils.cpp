#include <barcode/renderingutils.hpp>

#include <barcode/menustate.hpp>

#include <cstdlib>
#include <imgui/imgui.h>

#include <hydra/world/world.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>


#define frand() (float(rand())/RAND_MAX)

using world = Hydra::World::World;

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
		/*_fbo0->resize(size);
		_fbo1->resize(size);*/

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

	DefaultGraphicsPipeline::DefaultGraphicsPipeline(Hydra::System::CameraSystem& cameraSystem, const glm::ivec2& size) : IGraphicsPipeline(), _cameraSystem(cameraSystem), _blurUtil(_engine->getRenderer(), size / 2, Hydra::Renderer::TextureType::u8RGB) {
		_geometryBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/geometry.vert", "assets/shaders/geometry.geom", "assets/shaders/geometry.frag", size);
		_geometryBatch.output
			->addTexture(0, Hydra::Renderer::TextureType::f16RGB) // Position
			.addTexture(1, Hydra::Renderer::TextureType::u8RGBA) // Diffuse
			.addTexture(2, Hydra::Renderer::TextureType::f16RGB) // Normal
			.addTexture(3, Hydra::Renderer::TextureType::f16RGBA) // Light pos
			.addTexture(4, Hydra::Renderer::TextureType::u8RGB) // Position in view-space
			.addTexture(5, Hydra::Renderer::TextureType::u8R) // Glow.
			.addTexture(6, Hydra::Renderer::TextureType::f16Depth) // Depth
			.finalize();
		_geometryAnimationBatch = RenderBatch<Hydra::Renderer::AnimationBatch>("assets/shaders/animationGeometry.vert", "assets/shaders/animationGeometry.geom", "assets/shaders/animationGeometry.frag", _geometryBatch);
		_geometryAnimationBatch.batch.clearFlags = Hydra::Renderer::ClearFlags::none;

		_lightingBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/lighting.vert", "", "assets/shaders/lighting.frag", size);
		_lightingBatch.output
			->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
			.addTexture(1, Hydra::Renderer::TextureType::u8RGB)
			.finalize();

		_shadowBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/shadow.vert", "", "assets/shaders/shadow.frag", glm::vec2(512));
		_shadowBatch.output->addTexture(0, Hydra::Renderer::TextureType::f32Depth).finalize();
		_shadowBatch.batch.clearFlags = Hydra::Renderer::ClearFlags::depth;

		_shadowAnimationBatch = RenderBatch<Hydra::Renderer::AnimationBatch>("assets/shaders/shadowAnimation.vert", "", "assets/shaders/shadow.frag", _shadowBatch);
		_shadowAnimationBatch.batch.clearFlags = Hydra::Renderer::ClearFlags::none;


		_ssaoBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/ssao.vert", "", "assets/shaders/ssao.frag", size / 2);
		_ssaoBatch.output->addTexture(0, Hydra::Renderer::TextureType::f16R).finalize();
		_ssaoBlurBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/ssaoblur.vert", "", "assets/shaders/ssaoblur.frag", size/2);
		_ssaoBatch.output->addTexture(0, Hydra::Renderer::TextureType::f16R).finalize();

		constexpr size_t kernelSize = 16;
		constexpr size_t noiseSize = 4;
		auto ssaoKernel = _getSSAOKernel(kernelSize);
		for (size_t i = 0; i < ssaoKernel.size(); i++)
			_ssaoBatch.pipeline->setValue(4 + i, ssaoKernel[i]);

		_ssaoNoise = Hydra::Renderer::GLTexture::createFromData(noiseSize, noiseSize, Hydra::Renderer::TextureType::f32RGB, _getSSAONoise(noiseSize*noiseSize).data());


		_glowBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/glow.vert", "", "assets/shaders/glow.frag", _engine->getView());

		_copyBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/copy.vert", "", "assets/shaders/copy.frag", _engine->getView());

		_particleBatch = RenderBatch<Hydra::Renderer::ParticleBatch>("assets/shaders/particles.vert", "", "assets/shaders/particles.frag", _engine->getView());
		_particleAtlases = Hydra::Renderer::GLTexture::createFromFile("assets/textures/ParticleAtlases.png");
		_particleBatch.batch.clearFlags = ClearFlags::none;

		_textBatch = RenderBatch<Hydra::Renderer::TextBatch>("assets/shaders/text.vert", "", "assets/shaders/text.frag", _engine->getView());
		_textBatch.batch.clearFlags = ClearFlags::none;
	}

	DefaultGraphicsPipeline::~DefaultGraphicsPipeline() {
	}

	void DefaultGraphicsPipeline::render(const glm::vec3& cameraPos, Hydra::Component::CameraComponent& cc, Hydra::Component::TransformComponent& playerTransform) {
		auto& dirLight = *static_cast<Hydra::Component::LightComponent*>(Hydra::Component::LightComponent::componentHandler->getActiveComponents()[0].get());
		auto lightViewMX = dirLight.getViewMatrix();
		auto lightPMX = dirLight.getProjectionMatrix();
		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);
		glm::mat4 lightS = biasMatrix * lightPMX * lightViewMX;

		_geometryBatch.pipeline->setValue(0, cc.getViewMatrix());
		_geometryBatch.pipeline->setValue(1, cc.getProjectionMatrix());
		_geometryBatch.pipeline->setValue(2, cameraPos);
		_geometryBatch.pipeline->setValue(3, lightS);

		_geometryAnimationBatch.pipeline->setValue(0, cc.getViewMatrix());
		_geometryAnimationBatch.pipeline->setValue(1, cc.getProjectionMatrix());
		_geometryAnimationBatch.pipeline->setValue(2, cameraPos);
		_geometryAnimationBatch.pipeline->setValue(4, lightS);

		for (auto& kv : _geometryBatch.batch.objects)
			kv.second.clear();
		for (auto& kv : _geometryAnimationBatch.batch.objects)
			kv.second.clear();
		for (auto& kv : _geometryAnimationBatch.batch.currentFrames)
			kv.second.clear();
		for (auto& kv : _geometryAnimationBatch.batch.currAnimIndices)
			kv.second.clear();

		for (auto& kv : _shadowBatch.batch.objects)
			kv.second.clear();
		for (auto& kv : _shadowAnimationBatch.batch.objects)
			kv.second.clear();
		for (auto& kv : _shadowAnimationBatch.batch.currAnimIndices)
			kv.second.clear();
		for (auto& kv : _shadowAnimationBatch.batch.currentFrames)
			kv.second.clear();

		static bool enableFrustumCulling = false;
		//ImGui::Checkbox("Enable VF Culling", &enableFrustumCulling);

		constexpr float radius = 5.f;
		std::vector<std::shared_ptr<Entity>> entities;
		world::getEntitiesWithComponents<Hydra::Component::MeshComponent, Hydra::Component::DrawObjectComponent, Hydra::Component::TransformComponent>(entities);

		for (auto e : entities) {
			auto tc = e->getComponent<Hydra::Component::TransformComponent>();
			auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
			if (drawObj->disable || !drawObj->mesh)
				continue;

			bool renderNormal = true;
			if (enableFrustumCulling) {
				auto result = _cameraSystem.sphereInFrustum(tc->position, radius, cc);
				if (result == Hydra::System::CameraSystem::FrustrumCheck::outside)
					renderNormal = false;
			}

			if (drawObj->mesh->hasAnimation()) {
				auto mc = e->getComponent<Hydra::Component::MeshComponent>();

				if (renderNormal) {
					_geometryAnimationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
					_geometryAnimationBatch.batch.currentFrames[drawObj->mesh].push_back(mc->currentFrame);
					_geometryAnimationBatch.batch.currAnimIndices[drawObj->mesh].push_back(mc->animationIndex);
				}

				if (MenuState::shadowEnabled) {
					_shadowAnimationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
					_shadowAnimationBatch.batch.currentFrames[drawObj->mesh].push_back(mc->currentFrame);
					_shadowAnimationBatch.batch.currAnimIndices[drawObj->mesh].push_back(mc->animationIndex);
				}
			} else {
				if (renderNormal)
					_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
				
				if (MenuState::shadowEnabled && drawObj->hasShadow)
					_shadowBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
			}
		}

		_engine->getRenderer()->render(_geometryBatch.batch);
		_engine->getRenderer()->renderAnimation(_geometryAnimationBatch.batch);

		{
			_shadowBatch.pipeline->setValue(0, dirLight.getViewMatrix());
			_shadowBatch.pipeline->setValue(1, dirLight.getProjectionMatrix());

			_shadowAnimationBatch.pipeline->setValue(0, dirLight.getViewMatrix());
			_shadowAnimationBatch.pipeline->setValue(1, dirLight.getProjectionMatrix());

			_engine->getRenderer()->renderShadows(_shadowBatch.batch);
			_engine->getRenderer()->renderShadows(_shadowAnimationBatch.batch);
		}

		if (MenuState::ssaoEnabled) {
			_ssaoBatch.pipeline->setValue(0, 0);
			_ssaoBatch.pipeline->setValue(1, 1);
			_ssaoBatch.pipeline->setValue(2, 2);

			_ssaoBatch.pipeline->setValue(3, cc.getProjectionMatrix());

			(*_geometryBatch.output)[4]->bind(0);
			(*_geometryBatch.output)[2]->bind(1);
			_ssaoNoise->bind(2);

			_engine->getRenderer()->postProcessing(_ssaoBatch.batch);
		}

		{ // Lighting pass
			_lightingBatch.pipeline->setValue(0, 0);
			_lightingBatch.pipeline->setValue(1, 1);
			_lightingBatch.pipeline->setValue(2, 2);
			_lightingBatch.pipeline->setValue(3, 3);
			_lightingBatch.pipeline->setValue(4, 4);
			_lightingBatch.pipeline->setValue(5, 5);
			_lightingBatch.pipeline->setValue(6, 6);

			_lightingBatch.pipeline->setValue(7, playerTransform.position);
			_lightingBatch.pipeline->setValue(8, MenuState::ssaoEnabled);
			auto& lights = Hydra::Component::PointLightComponent::componentHandler->getActiveComponents();

			_lightingBatch.pipeline->setValue(9, (int)(lights.size()));
			_lightingBatch.pipeline->setValue(10, dirLight.getDirVec());
			_lightingBatch.pipeline->setValue(11, dirLight.color);


			// good code lmao XD
			int i = 12;
			for (auto& p : lights) {
				auto pc = static_cast<Hydra::Component::PointLightComponent*>(p.get());
				_lightingBatch.pipeline->setValue(i++, pc->getTransformComponent()->position);
				_lightingBatch.pipeline->setValue(i++, pc->color);
				_lightingBatch.pipeline->setValue(i++, pc->constant);
				_lightingBatch.pipeline->setValue(i++, pc->linear);
				_lightingBatch.pipeline->setValue(i++, pc->quadratic);
			}

			(*_geometryBatch.output)[0]->bind(0);
			(*_geometryBatch.output)[1]->bind(1);

			(*_geometryBatch.output)[2]->bind(2);
			(*_geometryBatch.output)[3]->bind(3);
			_shadowBatch.output->getDepth()->bind(4);
			(*_ssaoBatch.output)[0]->bind(5);
			(*_geometryBatch.output)[5]->bind(6);

			_engine->getRenderer()->postProcessing(_lightingBatch.batch);
		}

		if (MenuState::glowEnabled) {
			size_t nrOfTimes = 4;

			_blurUtil.blur((*_lightingBatch.output)[1], nrOfTimes, _lightingBatch.output->getSize() / 4);

			_glowBatch.batch.pipeline->setValue(1, 1);
			_glowBatch.batch.pipeline->setValue(2, 2);

			(*_lightingBatch.output)[0]->bind(1);
			_blurUtil.getOutput()->bind(2);
			_glowBatch.batch.pipeline->setValue(4, 4);
			_geometryBatch.output->getDepth()->bind(4);

			_engine->getRenderer()->postProcessing(_glowBatch.batch);
		} else {
			_copyBatch.batch.pipeline->setValue(1, 1);
			_copyBatch.batch.pipeline->setValue(2, 2);
			(*_lightingBatch.output)[0]->bind(1);
			_geometryBatch.output->getDepth()->bind(2);
			_engine->getRenderer()->postProcessing(_copyBatch.batch);
		}

		// Need it for my fruit salad down below (Text Rendering lmao)
		auto viewMatrix = cc.getViewMatrix();
		glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };

		{ // Particle batch
			for (auto& kv : _particleBatch.batch.objects) {
				kv.second.clear();
				_particleBatch.batch.textureInfo.clear();
			}
			std::vector<std::shared_ptr<Entity>> emitters;
			world::getEntitiesWithComponents<Hydra::Component::ParticleComponent>(emitters);
			for (auto& ee : emitters) { // Emitter Entities
				auto pc = ee->getComponent<Hydra::Component::ParticleComponent>();
				auto drawObj = ee->getComponent<Hydra::Component::DrawObjectComponent>();
				auto t = ee->getComponent<Hydra::Component::TransformComponent>();
				auto particles = pc->particles;
				for (size_t i = 0; i < Hydra::Component::ParticleComponent::MaxParticleAmount; i++) {
					if (particles[i].life <= 0)
						continue;
					_particleBatch.batch.objects[drawObj->drawObject->mesh].push_back(particles[i].getMatrix());
					_particleBatch.batch.textureInfo.push_back(particles[i].texOffset1);
					_particleBatch.batch.textureInfo.push_back(particles[i].texOffset2);
					_particleBatch.batch.textureInfo.push_back(particles[i].texCoordInfo);
				}
			}
			{
				_particleBatch.pipeline->setValue(0, viewMatrix);
				_particleBatch.pipeline->setValue(1, cc.getProjectionMatrix());
				_particleBatch.pipeline->setValue(2, rightVector);
				_particleBatch.pipeline->setValue(3, upVector);
				_particleBatch.pipeline->setValue(4, 0);
				_particleAtlases->bind(0);

				// Can't sort with objects need to sort per particle.
				//for (auto& kv : _particleBatch.batch.objects) {
				//	std::vector<glm::mat4>& list = kv.second;
				//
				//	std::sort(list.begin(), list.end(), [cameraPos](const glm::mat4& a, const glm::mat4& b) {
				//		return glm::distance(glm::vec3(a[3]), cameraPos) < glm::distance(glm::vec3(b[3]), cameraPos);
				//	});
				//}

				_engine->getRenderer()->render(_particleBatch.batch);
			}
		}

		{ // Text render batch.
			for (auto& kv : _textBatch.batch.objects)
				kv.second.clear();

			_textBatch.batch.textInfo.clear();
			_textBatch.batch.textSizes.clear();
			_textBatch.batch.lifeFade.clear();

			std::vector<std::shared_ptr<Entity>> entities;
			world::getEntitiesWithComponents<Hydra::Component::TextComponent, Hydra::Component::DrawObjectComponent>(entities);
			for (auto e : entities) {
				auto textC = e->getComponent<Hydra::Component::TextComponent>();
				auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
				auto textData = textC->renderingData;

				for (size_t i = 0; i < textData.size(); i++)
					_textBatch.batch.textInfo.push_back(textC->renderingData[i]);

				auto lifeC = e->getComponent<Hydra::Component::LifeComponent>();
				if (lifeC)
					_textBatch.batch.lifeFade.push_back(e->getComponent<Hydra::Component::LifeComponent>()->health);
				else
					_textBatch.batch.lifeFade.push_back(1.0f);

				_textBatch.batch.textSizes.push_back(textData.size());
				_textBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
			}
			_textBatch.pipeline->setValue(0, cc.getProjectionMatrix() * cc.getViewMatrix());
			_textBatch.pipeline->setValue(1, 0);
			_textBatch.pipeline->setValue(3, rightVector);
			_textBatch.pipeline->setValue(4, upVector);
			_engine->getState()->getTextFactory()->getTexture()->bind(0);
			_engine->getRenderer()->renderText(_textBatch.batch);
		}

	}

	std::vector<glm::vec3> DefaultGraphicsPipeline::_getSSAOKernel(size_t size) {
		std::vector<glm::vec3> ssaoKernel;
		ssaoKernel.resize(size);
		for (size_t i = 0; i < size; i++) {
			float scale = (float)i / size;
			scale = 0.1 + (scale * scale) * (1.0 - 0.1);
			float x = frand() * 2.0 - 1.0;
			float y = frand() * 2.0 - 1.0;
			float z = frand();
			ssaoKernel[i] = glm::normalize(glm::vec3{x, y, z}) * frand() * scale;
		}
		return ssaoKernel;
	}
	std::vector<glm::vec3> DefaultGraphicsPipeline::_getSSAONoise(size_t size) {
		std::vector<glm::vec3> ssaoNoise;
		ssaoNoise.resize(size);
		for (size_t i = 0; i < size; i++) {
			float x = frand() * 2.0 - 1.0;
			float y = frand() * 2.0 - 1.0;
			ssaoNoise[i] = glm::vec3{x, y, 0.0f};
		}
		return ssaoNoise;
	}
}
