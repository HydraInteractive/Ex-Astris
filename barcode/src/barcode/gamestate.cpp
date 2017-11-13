#include <barcode/gamestate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>
#include <barcode/tileGeneration.hpp>

#include <glm/gtx/matrix_decompose.hpp>

#include <hydra/world/blueprintloader.hpp>
#include <imgui/imgui.h>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>

using world = Hydra::World::World;

namespace Barcode {
	GameState::GameState() : _engine(Hydra::IEngine::getInstance()) {}

	void GameState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		auto windowSize = _engine->getView()->getSize();
		{
			auto& batch = _geometryBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/geometry.vert");
			batch.geometryShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::geometry, "assets/shaders/geometry.geom");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/geometry.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.geometryShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::f16RGB) // Position
				.addTexture(1, Hydra::Renderer::TextureType::u8RGBA) // Diffuse
				.addTexture(2, Hydra::Renderer::TextureType::f16RGB) // Normal
				.addTexture(3, Hydra::Renderer::TextureType::f16RGBA) // Light pos
				.addTexture(4, Hydra::Renderer::TextureType::u8RGB) // Position in view-space
				.addTexture(5, Hydra::Renderer::TextureType::u8R) // Glow.
				.addTexture(6, Hydra::Renderer::TextureType::f16Depth) // real depth
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{
			auto& batch = _animationBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/animationGeometry.vert");
			batch.geometryShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::geometry, "assets/shaders/animationGeometry.geom");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/animationGeometry.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.geometryShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::none;
			batch.batch.renderTarget = _geometryBatch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{ // Lighting pass batch
			auto& batch = _lightingBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/lighting.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/lighting.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.addTexture(1, Hydra::Renderer::TextureType::u8RGB)
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get(); // TODO: Change to "null" pipeline
		}

		{
			auto& batch = _glowBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/blur.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/blur.frag");

			_glowVertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/glow.vert");
			_glowFragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/glow.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			_glowPipeline = Hydra::Renderer::GLPipeline::create();
			_glowPipeline->attachStage(*_glowVertexShader);
			_glowPipeline->attachStage(*_glowFragmentShader);
			_glowPipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{
			// Extra buffer for ping-ponging the texture for two-pass gaussian blur.
			_blurrExtraFBO1 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			_blurrExtraFBO1
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();
			_blurrExtraFBO2 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			_blurrExtraFBO2
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();
		}

		{ // PARTICLES
			auto& batch = _particleBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/particles.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/particles.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			_particleAtlases = Hydra::Renderer::GLTexture::createFromFile("assets/textures/ParticleAtlases.png");

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::none;
			batch.batch.renderTarget = _engine->getView();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{ // Shadow pass
			auto& batch = _shadowBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/shadow.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/shadow.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(glm::vec2(1024), 0);
			batch.output->addTexture(0, Hydra::Renderer::TextureType::f16Depth).finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();

			auto& animBatch = _shadowAnimationBatch;
			animBatch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/shadowAnimation.vert");
			animBatch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/shadow.frag");

			animBatch.pipeline = Hydra::Renderer::GLPipeline::create();
			animBatch.pipeline->attachStage(*animBatch.vertexShader);
			animBatch.pipeline->attachStage(*animBatch.fragmentShader);
			animBatch.pipeline->finalize();

			animBatch.batch.clearColor = glm::vec4(0, 0, 0, 0);
			animBatch.batch.clearFlags = Hydra::Renderer::ClearFlags::none;
			animBatch.batch.renderTarget = batch.output.get();
			animBatch.batch.pipeline = animBatch.pipeline.get();
		}

		{ // SSAO
			auto& batch = _ssaoBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/ssao.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/ssao.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(windowSize / 2, 0);
			batch.output->addTexture(0, Hydra::Renderer::TextureType::f16R).finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();

			std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
			std::default_random_engine generator;
			size_t kernelSize = 8;
			std::vector<glm::vec3> ssaoKernel;
			for (size_t i = 0; i < kernelSize; i++) {
				glm::vec3 sample(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator)
				);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = (float)i / kernelSize;
				scale = 0.1 + (scale * scale) * (1.0 - 0.1);
				sample *= scale;
				ssaoKernel.push_back(sample);
			}

			std::vector<glm::vec3> ssaoNoise;
			for (unsigned int i = 0; i < 16; i++) {
				glm::vec3 noise(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					0.0f);
				ssaoNoise.push_back(noise);
			}

			_ssaoNoise = Hydra::Renderer::GLTexture::createFromData(4, 4, TextureType::f32RGB, ssaoNoise.data());

			for (size_t i = 0; i < kernelSize; i++)
				_ssaoBatch.pipeline->setValue(4 + i, ssaoKernel[i]);
		}

		{
			auto& batch = _viewBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/view.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/view.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0, 0.0, 0.0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = _engine->getView();
			batch.batch.pipeline = batch.pipeline.get(); // TODO: Change to "null" pipeline
		}

		{
			auto& batch = _hitboxBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/hitboxdebug.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/hitboxdebug.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::none;
			batch.batch.renderTarget = _lightingBatch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		size_t boneCount = 32;
		size_t animationBatch = 16;
		// TODO: Change to f16?
		_animationData = Hydra::Renderer::GLTexture::createDataTexture(boneCount * 4, animationBatch, Hydra::Renderer::TextureType::f32RGBA);

		_initWorld();
	}

	GameState::~GameState() { }

	void GameState::onMainMenu() { }

	void GameState::runFrame(float delta) {
		auto windowSize = _engine->getView()->getSize();

		if (ImGui::Button("Remove unused meshes")) 
			_meshLoader->clear();

		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		_playerSystem.tick(delta);
		_abilitySystem.tick(delta);
		_lightSystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);
		_spawnerSystem.tick(delta);
		_soundFxSystem.tick(delta);
		_perkSystem.tick(delta);
		_lifeSystem.tick(delta);

		//TODO: These should go straight to the transform component, not via the camera component
		const glm::vec3 cameraPos = _cc->getTransformComponent()->position;

		{ // Render objects (Deferred rendering)
		  //_world->tick(TickAction::render, delta);

		  // Render to geometryFBO

		  // FIXME: Fix this shit code
			for (auto& light : Hydra::Component::LightComponent::componentHandler->getActiveComponents())
				_light = static_cast<Hydra::Component::LightComponent*>(light.get());

			auto lightViewMX = _light->getViewMatrix();
			auto lightPMX = _light->getProjectionMatrix();
			glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
			);
			glm::mat4 lightS = biasMatrix * lightPMX * lightViewMX;

			_geometryBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_geometryBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_geometryBatch.pipeline->setValue(2, cameraPos);
			_geometryBatch.pipeline->setValue(4, lightS);

			_animationBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_animationBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_animationBatch.pipeline->setValue(2, cameraPos);
			_animationBatch.pipeline->setValue(4, lightS);

			for (auto& kv : _geometryBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _shadowBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _animationBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _animationBatch.batch.currentFrames)
				kv.second.clear();

			for (auto& kv : _animationBatch.batch.currAnimIndices)
				kv.second.clear();

			for (auto& kv : _shadowAnimationBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _shadowAnimationBatch.batch.currAnimIndices)
				kv.second.clear();

			for (auto& kv : _shadowAnimationBatch.batch.currentFrames)
				kv.second.clear();

			static bool enableFrustumCulling = true;

			ImGui::Checkbox("Enable VF Culling", &enableFrustumCulling);
			if (enableFrustumCulling) {
				float radius = 5.f;
				std::vector<std::shared_ptr<Entity>> entities;
				world::getEntitiesWithComponents<Hydra::Component::MeshComponent, Hydra::Component::DrawObjectComponent, Hydra::Component::TransformComponent>(entities);

				auto viewMatrix = _cc->getViewMatrix();
				glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
				glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
				glm::vec3 dir = glm::cross(rightVector, upVector);
				_cameraSystem.setCamInternals(*_cc);
				_cameraSystem.setCamDef(_cc->getTransformComponent()->position, dir, upVector, rightVector, *_cc);

				for (auto e : entities) {
					auto tc = e->getComponent<Hydra::Component::TransformComponent>();
					auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
					int result = _cameraSystem.sphereInFrustum(tc->position, radius, *_cc);
					if (result == _cc->INSIDE || result == _cc->INTERSECT) {
						if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == false) {
							_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
							_shadowBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
						}

						else if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == true) {
							auto mc = e->getComponent<Hydra::Component::MeshComponent>();
							int currentFrame = mc->currentFrame;
							float animationCounter = mc->animationCounter;

							if (animationCounter > 1 / 24.0f && currentFrame < mc->mesh->getMaxFramesForAnimation(mc->animationIndex)) {
								mc->animationCounter -= 1 / 24.0f;
								mc->currentFrame += 1;
							}
							else if (currentFrame >= mc->mesh->getMaxFramesForAnimation(mc->animationIndex))
								mc->currentFrame = 1;

							_animationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
							_animationBatch.batch.currentFrames[drawObj->mesh].push_back(mc->currentFrame);
							_animationBatch.batch.currAnimIndices[drawObj->mesh].push_back(mc->animationIndex);
							_shadowAnimationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
							_shadowAnimationBatch.batch.currentFrames[drawObj->mesh].push_back(mc->currentFrame);
							_shadowAnimationBatch.batch.currAnimIndices[drawObj->mesh].push_back(mc->animationIndex);
							mc->animationCounter += 1 * delta;
						}
					}
				}
			}
			else {
				// This is so stupid; someone save this code I need to do other stuff.
				for (auto& drawObj : _engine->getRenderer()->activeDrawObjects()) {
					if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == false) {
						_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
						_shadowBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
					}
				}

				// Should fix so drawobject has pointer to entity so i don't have to use getEntitiesWithComponents since it unnecessary
				std::vector<std::shared_ptr<Hydra::World::Entity>> meshEntities;
				world::getEntitiesWithComponents<Hydra::Component::MeshComponent, Hydra::Component::DrawObjectComponent, Hydra::Component::TransformComponent>(meshEntities);
				int i = 0;
				for (auto e : meshEntities) {
					auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
					auto mesh = drawObj->mesh;
					if (mesh->hasAnimation() == false || drawObj->disable || !drawObj->mesh)
						continue;


					auto mc = e->getComponent<Hydra::Component::MeshComponent>();
					int currentFrame = mc->currentFrame;
					float animationCounter = mc->animationCounter;

					if (animationCounter > 1 / 24.0f && currentFrame < mc->mesh->getMaxFramesForAnimation(mc->animationIndex)) {
						mc->animationCounter -= 1 / 24.0f;
						mc->currentFrame += 1;
					}
					else if (currentFrame >= mc->mesh->getMaxFramesForAnimation(mc->animationIndex))
						mc->currentFrame = 1;

					_animationBatch.batch.objects[mesh].push_back(drawObj->modelMatrix);
					_animationBatch.batch.currentFrames[mesh].push_back(mc->currentFrame);
					_animationBatch.batch.currAnimIndices[mesh].push_back(mc->animationIndex);
					_shadowAnimationBatch.batch.objects[mesh].push_back(drawObj->modelMatrix);
					_shadowAnimationBatch.batch.currentFrames[mesh].push_back(mc->currentFrame);
					_shadowAnimationBatch.batch.currAnimIndices[mesh].push_back(mc->animationIndex);
					mc->animationCounter += 1 * delta;
				}
			}
			_engine->getRenderer()->render(_geometryBatch.batch);
			_engine->getRenderer()->renderAnimation(_animationBatch.batch);
		}

		{
			_shadowBatch.pipeline->setValue(0, _light->getViewMatrix());
			_shadowBatch.pipeline->setValue(1, _light->getProjectionMatrix());

			_shadowAnimationBatch.pipeline->setValue(0, _light->getViewMatrix());
			_shadowAnimationBatch.pipeline->setValue(1, _light->getProjectionMatrix());

			_engine->getRenderer()->renderShadows(_shadowBatch.batch);
			_engine->getRenderer()->renderShadows(_shadowAnimationBatch.batch);
		}

		static bool enableSSAO = false;
		ImGui::Checkbox("Enable SSAO", &enableSSAO);
		static bool enableBlur = true;
		ImGui::Checkbox("Enable blur", &enableBlur);
		static bool enableHitboxDebug = true;
		ImGui::Checkbox("Enable Hitbox Debug", &enableHitboxDebug);

		if (enableSSAO) {
			_ssaoBatch.pipeline->setValue(0, 0);
			_ssaoBatch.pipeline->setValue(1, 1);
			_ssaoBatch.pipeline->setValue(2, 2);

			_ssaoBatch.pipeline->setValue(3, _cc->getProjectionMatrix());

			(*_geometryBatch.output)[4]->bind(0);
			(*_geometryBatch.output)[2]->bind(1);
			_ssaoNoise->bind(2);

			_engine->getRenderer()->postProcessing(_ssaoBatch.batch);
			int nrOfTimes = 1;
			_blurGlowTexture((*_ssaoBatch.output)[0], nrOfTimes, (*_ssaoBatch.output)[0]->getSize());
		}

		{ // Lighting pass
			_lightingBatch.pipeline->setValue(0, 0);
			_lightingBatch.pipeline->setValue(1, 1);
			_lightingBatch.pipeline->setValue(2, 2);
			_lightingBatch.pipeline->setValue(3, 3);
			_lightingBatch.pipeline->setValue(4, 4);
			_lightingBatch.pipeline->setValue(5, 5);
			_lightingBatch.pipeline->setValue(6, 6);

			_lightingBatch.pipeline->setValue(7, _cc->getTransformComponent()->position);
			_lightingBatch.pipeline->setValue(8, enableSSAO);
			auto& lights = Hydra::Component::PointLightComponent::componentHandler->getActiveComponents();

			_lightingBatch.pipeline->setValue(9, (int)(lights.size()));
			_lightingBatch.pipeline->setValue(10, _light->getDirVec());
			_lightingBatch.pipeline->setValue(11, _light->color);


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
			(*_blurrExtraFBO1)[0]->bind(5);
			(*_geometryBatch.output)[5]->bind(6);

			_engine->getRenderer()->postProcessing(_lightingBatch.batch);
		}
		if (enableHitboxDebug) {
			for (auto& kv : _hitboxBatch.batch.objects) {
				kv.second.clear();
			}

			std::vector<std::shared_ptr<Entity>> entities;
			world::getEntitiesWithComponents<Hydra::Component::RigidBodyComponent, Hydra::Component::DrawObjectComponent>(entities);
			for (auto e : entities) {
				// GOTTA MAKE IT VERSATILE SO IT CAN TAKE CAPSULE AS WELL.
				auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
				auto rgbc = e->getComponent<Hydra::Component::RigidBodyComponent>();
				glm::vec3 newScale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(drawObj->modelMatrix, newScale, rotation, translation, skew, perspective);
				_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(rgbc->getHalfExtentScale() * glm::vec3(2)));
			}
			_hitboxBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_hitboxBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_engine->getRenderer()->renderHitboxes(_hitboxBatch.batch);
		}
		{ // Glow
			if (enableBlur) {
				int nrOfTimes;
				nrOfTimes = 4;

				glm::vec2 size = windowSize;

				_blurGlowTexture((*_lightingBatch.output)[1], nrOfTimes, size * 0.25f);

				_glowBatch.batch.pipeline = _glowPipeline.get();

				_glowBatch.batch.pipeline->setValue(1, 1);
				_glowBatch.batch.pipeline->setValue(2, 2);

				(*_lightingBatch.output)[0]->bind(1);
				(*_blurrExtraFBO1)[0]->bind(2);
				_glowBatch.batch.pipeline->setValue(4, 4);
				_geometryBatch.output->getDepth()->bind(4);

				_glowBatch.batch.renderTarget = _engine->getView();
				_engine->getRenderer()->postProcessing(_glowBatch.batch);
				_glowBatch.batch.renderTarget = _glowBatch.output.get();
				_glowBatch.batch.pipeline = _glowBatch.pipeline.get();
			}
			else
				_engine->getView()->blit(_lightingBatch.output.get());
		}

		{ // Render transparent objects	(Forward rendering)
		  //_world->tick(TickAction::renderTransparent, delta);
		}

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
				for (int i = 0; i < Hydra::Component::ParticleComponent::MaxParticleAmount; i++) {
					if (particles[i].life <= 0)
						continue;
					_particleBatch.batch.objects[drawObj->drawObject->mesh].push_back(particles[i].getMatrix());
					_particleBatch.batch.textureInfo.push_back(particles[i].texOffset1);
					_particleBatch.batch.textureInfo.push_back(particles[i].texOffset2);
					_particleBatch.batch.textureInfo.push_back(particles[i].texCoordInfo);
				}
			}
			{
				auto viewMatrix = _cc->getViewMatrix();
				glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
				glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
				_particleBatch.pipeline->setValue(0, viewMatrix);
				_particleBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
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

		{ // Hud windows
		  //static float f = 0.0f;
		  //static bool b = false;
		  //static float invisF[3] = { 0, 0, 0 };
			float hpP = 100;
			float ammoP = 100;
			float degrees = 0;
			//std::vector<Buffs> perksList;
			for (auto& p : Hydra::Component::PlayerComponent::componentHandler->getActiveComponents()) {
				auto player = static_cast<Hydra::Component::PlayerComponent*>(p.get());
				//perksList = player->activeBuffs.getActiveBuffs();
			}
			for (auto& camera : Hydra::Component::CameraComponent::componentHandler->getActiveComponents())
				degrees = glm::degrees(static_cast<Hydra::Component::CameraComponent*>(camera.get())->cameraYaw);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, float(0.0f));

			const int x = _engine->getView()->getSize().x / 2;
			const ImVec2 pos = ImVec2(x, _engine->getView()->getSize().y / 2);

			//Crosshair
			ImGui::SetNextWindowPos(pos + ImVec2(-10, 1));
			ImGui::SetNextWindowSize(ImVec2(20, 20));
			ImGui::Begin("Crosshair", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Crosshair.png")->getID()), ImVec2(20, 20));
			ImGui::End();

			//AimRing
			ImGui::SetNextWindowPos(pos + ImVec2(-51, -42));
			ImGui::SetNextWindowSize(ImVec2(120, 120));
			ImGui::Begin("AimRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AimRing.png")->getID()), ImVec2(100, 100));
			ImGui::End();

			//Hp bar on ring
			float offsetHpF = 72 * hpP * 0.01;
			int offsetHp = offsetHpF;
			ImGui::SetNextWindowPos(pos + ImVec2(-47, -26 + 72 - offsetHp));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("HpOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/HpOnRing.png")->getID()), ImVec2(22, offsetHp), ImVec2(0, 1 - hpP * 0.01), ImVec2(1, 1));
			ImGui::End();

			//Ammo on bar
			float offsetAmmoF = 72 * ammoP * 0.01;
			size_t offsetAmmo = offsetAmmoF;
			ImGui::SetNextWindowPos(pos + ImVec2(+25, -26 + 72 - offsetAmmo));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("AmmoOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AmmoOnRing.png")->getID()), ImVec2(22, offsetAmmo), ImVec2(0, 1 - ammoP * 0.01), ImVec2(1, 1));
			ImGui::End();

			//compas that turns with player
			float degreesP = ((float(100) / float(360) * degrees) / 100);
			float degreesO = float(1000) * degreesP;
			ImGui::SetNextWindowPos(ImVec2(pos.x - 275, +70));
			ImGui::SetNextWindowSize(ImVec2(600, 20));
			ImGui::Begin("Compass", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/CompassCut.png")->getID()), ImVec2(550, 20), ImVec2(degreesO / float(1000), 0), ImVec2((float(1) - ((float(450) - degreesO) / float(1000))), 1));
			_textureLoader->getTexture("assets/hud/CompassCut.png")->setRepeat();
			ImGui::End();

			//Enemys on compas
			int i = 0;
			glm::mat4 viewMat = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get())->getViewMatrix();
			std::vector<std::shared_ptr<Entity>> aiEntities;
			world::getEntitiesWithComponents<Hydra::Component::AIComponent>(aiEntities);
			for (auto& enemy : aiEntities) {
				char buf[128];
				snprintf(buf, sizeof(buf), "AI is a scrub here is it's scrubID: %d", i);
				auto playerP = _cc->getTransformComponent()->position;
				auto enemyP = enemy->getComponent<Hydra::Component::TransformComponent>()->position;
				auto enemyDir = glm::normalize(enemyP - playerP);

				glm::vec3 forward(-viewMat[0][2], -viewMat[1][2], -viewMat[2][2]);
				glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

				glm::vec2 forward2D = glm::normalize(glm::vec2(forward.x, forward.z));
				glm::vec2 right2D = glm::normalize(glm::vec2(right.x, right.z));
				glm::vec2 enemy2D = glm::normalize(glm::vec2(enemyDir.x, enemyDir.z));

				float dotPlacment = glm::dot(forward2D, enemy2D); // -1 - +1
				float leftRight = glm::dot(right2D, enemy2D);
				if (leftRight < 0)
				{
					leftRight = 1;
				}
				else
				{
					leftRight = -1;
				}
				if (dotPlacment < 0)
				{
					dotPlacment = 0;
				}
				dotPlacment = dotPlacment;
				dotPlacment = leftRight * (1 - dotPlacment) * 275;
				ImGui::SetNextWindowPos(ImVec2(x + dotPlacment, 75)); //- 275
				ImGui::SetNextWindowSize(ImVec2(20, 20));
				ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
				ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Red.png")->getID()), ImVec2(10, 10));
				ImGui::End();
				i++;
			}

			//Dynamic cooldown dots
			int amountOfActives = 3;
			int coolDownList[64] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
			float pForEatchDot = float(1) / float(amountOfActives);
			float stepSize = float(70) * pForEatchDot;
			for (int i = 0; i < amountOfActives; i++)
			{
				char buf[128];
				snprintf(buf, sizeof(buf), "Cooldown%d", i);
				float yOffset = float(stepSize * float(i + 1));
				float xOffset = pow(abs((yOffset - (stepSize / float(2)) - float(35))) * 0.1069, 2);
				ImGui::SetNextWindowPos(pos + ImVec2(-64 + xOffset, -24 + yOffset - ((stepSize + 10.0) / 2.0)));
				ImGui::SetNextWindowSize(ImVec2(15, 15));
				ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
				if (coolDownList[i] >= 7)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Red.png")->getID()), ImVec2(10, 10));
				}
				else if (coolDownList[i] <= 0)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Green.png")->getID()), ImVec2(10, 10));
				}
				else
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Yellow.png")->getID()), ImVec2(10, 10));
				}

				ImGui::End();
			}

			//Perk Icons
			/*size_t amountOfPerks = perksList.size();
			for (size_t i = 0; i < amountOfPerks; i++)
			{
				char buf[128];
				snprintf(buf, sizeof(buf), "Perk%lu", i);
				float xOffset = float((-10 * amountOfPerks) + (20 * i));
				ImGui::SetNextWindowPos(pos + ImVec2(xOffset, +480));
				ImGui::SetNextWindowSize(ImVec2(20, 20));
				ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
				switch (perksList[i])
				{
				case BUFF_BULLETVELOCITY:
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/BulletVelocity.png")->getID()), ImVec2(20, 20));
					break;
				case BUFF_DAMAGEUPGRADE:
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/DamageUpgrade.png")->getID()), ImVec2(20, 20));
					break;
				case BUFF_HEALING:
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Healing.png")->getID()), ImVec2(20, 20));
					break;
				case BUFF_HEALTHUPGRADE:
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/HealthUpgrade.png")->getID()), ImVec2(20, 20));
					break;
				}

				ImGui::End();
			}*/

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			//////Debug for pathfinding
			//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, float(0.0f));
			//int k = 0;
			//for (auto& entity : _world->getActiveComponents<Hydra::Component::AIComponent>())
			//{
			//	for (int i = 0; i < 30; i++)
			//	{
			//		for (int j = 0; j < 30; j++)
			//		{
			//			if (entity != nullptr)
			//			{
			//				char buf[128];
			//				snprintf(buf, sizeof(buf), "%d%d", i, j);
			//				if (entity->getComponent<Hydra::Component::AIComponent>()->getWall(i, j) == 1)
			//				{
			//					ImGui::SetNextWindowPos(ImVec2(10 * i, 10 * j));
			//					ImGui::SetNextWindowSize(ImVec2(20, 20));
			//					ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			//					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Red.png")->getID()), ImVec2(20, 20));
			//					ImGui::End();
			//				}
			//				if (entity->getComponent<Hydra::Component::AIComponent>()->getWall(i, j) == 2)
			//				{
			//					ImGui::SetNextWindowPos(ImVec2(10 * i, 10 * j));
			//					ImGui::SetNextWindowSize(ImVec2(20, 20));
			//					ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			//					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Blue.png")->getID()), ImVec2(20, 20));
			//					ImGui::End();
			//				}
			//				else if (entity->getComponent<Hydra::Component::AIComponent>()->getWall(i, j) == 3)
			//				{
			//					ImGui::SetNextWindowPos(ImVec2(10 * i, 10 * j));
			//					ImGui::SetNextWindowSize(ImVec2(20, 20));
			//					ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			//					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Yellow.png")->getID()), ImVec2(20, 20));
			//					ImGui::End();
			//				}
			//				//else if (entity->getComponent<Hydra::Component::AIComponent>()->getWall(i, j) == 0)
			//				//{
			//				//	ImGui::SetNextWindowPos(ImVec2(10 * i, 10 * j));
			//				//	ImGui::SetNextWindowSize(ImVec2(20, 20));
			//				//	ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			//				//	ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Green.png")->getID()), ImVec2(20, 20));
			//				//	ImGui::End();
			//				//}
			//			}
			//		}
			//	}
			//	k++;
			//}
			//ImGui::PopStyleColor();
			//ImGui::PopStyleVar();
			//ImGui::PopStyleVar();
		}

		{ // Sync with network
		  // _world->tick(TickAction::network, delta);
		}
	}

	void GameState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_lightSystem, &_particleSystem, &_abilitySystem, &_aiSystem, &_physicsSystem, &_bulletSystem, &_playerSystem, &_rendererSystem, &_spawnerSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void GameState::_initWorld() {
		_hitboxCube = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getMesh("assets/objects/HitBox.mATTIC");
		{
			auto floor = world::newEntity("Floor", world::root());
			auto t = floor->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(0, -7, 0);
			auto rgbc = floor->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createStaticPlane(glm::vec3(0, 1, 0), 1, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_WALL
				, 0, 0, 0, 0.6f, 0);
			floor->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Floor_v2.mATTIC");
		}
		{
			TileGeneration worldTiles("assets/room/threewayRoom.room");
		}
		{
			auto physicsBox = world::newEntity("Physics box", world::root());
			auto t = physicsBox->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(2, 25, 2);
			physicsBox->addComponent<Hydra::Component::RigidBodyComponent>()->createBox(t->scale * 10.0f, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_MISC_OBJECT, 10
				, 0, 0, 1.0f, 1.0f);
			physicsBox->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BigMonitor.mATTIC");
		}

		{
			auto playerEntity = world::newEntity("Player", world::root());
			auto p = playerEntity->addComponent<Hydra::Component::PlayerComponent>();
			auto c = playerEntity->addComponent<Hydra::Component::CameraComponent>();
			auto h = playerEntity->addComponent<Hydra::Component::LifeComponent>();
			auto m = playerEntity->addComponent<Hydra::Component::MovementComponent>();
			auto s = playerEntity->addComponent<Hydra::Component::SoundFxComponent>();
			auto perks = playerEntity->addComponent<Hydra::Component::PerkComponent>();
			h->health = h->maxHP = 2000.0f;
			m->movementSpeed = 20.0f;
			//c->position = glm::vec3{ 5, 0, -3 };
			auto t = playerEntity->addComponent<Hydra::Component::TransformComponent>();
			auto rgbc = playerEntity->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(0.5f * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER, 100,
				0, 0, 0.5f, 0);
			rgbc->setActivationState(DISABLE_DEACTIVATION);
			t->position = glm::vec3{ 0, -7, 20 };
			{
				auto weaponEntity = world::newEntity("Weapon", playerEntity);
				weaponEntity->addComponent<Hydra::Component::WeaponComponent>();
				weaponEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Gun.mATTIC");
				auto t2 = weaponEntity->addComponent<Hydra::Component::TransformComponent>();
				t2->position = glm::vec3(2, -7, -2);
				t2->rotation = glm::quat(0, 0, 1, 0);
				t2->ignoreParent = true;
			}
		}

		for (int i = 0; i < 100; i++) {
			{
				auto alienEntity = world::newEntity("Alien" + std::to_string(i), world::root());
				alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
				auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
				a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
				a->damage = 4;
				a->behaviour->originalRange = 4;
				a->radius = 1;

				auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
				h->maxHP = 80;
				h->health = 80;
				auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
				m->movementSpeed = 8.0f;
				auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3{ i * 5, 0, i * 5};
				t->scale = glm::vec3{ 2,2,2 };
				t->rotation = glm::vec3{ 0, 90, 0 };
				auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
				rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
					0, 0, 0.6f, 1.0f);
				rgbc->setActivationState(DISABLE_DEACTIVATION);
			}

		}

		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/PlayerModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}

		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien1", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 1;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;
		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 8.0f;
		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 0, 0, 5 };
		//	t->scale = glm::vec3{ 2,2,2 };
		//	t->rotation = glm::vec3{ 0, 90, 0 };
		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}


		//{
		//	auto alienEntity = world::newEntity("Alien3", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 2;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;

		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 4.0f;

		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 25, 0, 20 };
		//	t->scale = glm::vec3{ 2,2,2 };

		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien4", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 2;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;

		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 4.0f;

		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 5, 0, 20 };
		//	t->scale = glm::vec3{ 2,2,2 };

		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien4", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 2;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;

		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 4.0f;

		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 5, 0, 20 };
		//	t->scale = glm::vec3{ 2,2,2 };

		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien4", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 2;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;

		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 4.0f;

		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 5, 0, 20 };
		//	t->scale = glm::vec3{ 2,2,2 };

		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}
		//{
		//	auto alienEntity = world::newEntity("Alien4", world::root());
		//	alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		//	auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		//	a->damage = 4;
		//	a->behaviour->originalRange = 4;
		//	a->radius = 2;

		//	auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		//	h->maxHP = 80;
		//	h->health = 80;

		//	auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 4.0f;

		//	auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3{ 5, 0, 20 };
		//	t->scale = glm::vec3{ 2,2,2 };

		//	auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
		//		0, 0, 0.6f, 1.0f);
		//	rgbc->setActivationState(DISABLE_DEACTIVATION);
		//}

		{
			auto pointLight1 = world::newEntity("Pointlight1", world::root());
			pointLight1->addComponent<Hydra::Component::TransformComponent>();
			pointLight1->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/EscapePodDoor.mATTIC");
			auto p1LC = pointLight1->addComponent<Hydra::Component::PointLightComponent>();
			p1LC->color = glm::vec3(1, 1, 1);
		} {
			auto pointLight2 = world::newEntity("Pointlight2", world::root());
			auto t = pointLight2->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(45, 0, 0);
			pointLight2->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
			auto p2LC = pointLight2->addComponent<Hydra::Component::PointLightComponent>();
			p2LC->color = glm::vec3(1, 1, 1);
		} {
			auto pointLight3 = world::newEntity("Pointlight3", world::root());
			auto t = pointLight3->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(45, 0, 0);
			pointLight3->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
			auto p3LC = pointLight3->addComponent<Hydra::Component::PointLightComponent>();
			p3LC->color = glm::vec3(1, 1, 1);
		}

		{
			auto pointLight4 = world::newEntity("Pointlight4", world::root());
			auto t = pointLight4->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(45, 0, 0);
			pointLight4->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
			auto p4LC = pointLight4->addComponent<Hydra::Component::PointLightComponent>();
			p4LC->color = glm::vec3(1, 1, 1);
		}

		{

			{
				auto parent = world::newEntity("Parent", world::root());
				auto tp = parent->addComponent<Hydra::Component::TransformComponent>();
				tp->position = glm::vec3{ 0, 0, 10 };
				parent->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BigMonitor.mATTIC");

				{
					auto child = world::newEntity("child", parent);
					auto t = child->addComponent<Hydra::Component::TransformComponent>();
					t->position = glm::vec3{ 1, 0, 0 };
					child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");
				}
				{
					auto child = world::newEntity("child", parent);
					auto t = child->addComponent<Hydra::Component::TransformComponent>();
					t->position = glm::vec3{ -1, 0, 0 };
					child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");
				}
			}

			{
				auto particleEmitter = world::newEntity("ParticleEmitter", world::root());
				particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("QUAD");
				auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
				p->delay = 1.0f / 1.0f;
				auto t = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3{ 4, 0, 4 };
			}

			{
				auto lightEntity = world::newEntity("Light", world::root());
				auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
				auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3(8.0, 0, 3.5);
			}

			//TODO: Fix AI Serialization
			//{
			//	BlueprintLoader::save("world.blueprint", "World Blueprint", world::root());
			//	Hydra::World::World::reset();
			//	auto bp = BlueprintLoader::load("world.blueprint");
			//	bp->spawn(world::root());
			//}

			{
				_cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());

				for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents()) {
					_engine->log(Hydra::LogLevel::normal, "Enabling bullet for %s", world::getEntity(rb->entityID)->name.c_str());
					_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
				}
			}
		}
	}

	std::shared_ptr<Hydra::Renderer::IFramebuffer> GameState::_blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int nrOfTimes, glm::vec2 size) {
		// TO-DO: Make it agile so it can blur any texture
		_glowBatch.pipeline->setValue(1, 1); // This bind will never change
		bool horizontal = true;
		bool firstPass = true;
		_blurrExtraFBO1->resize(size);
		_blurrExtraFBO2->resize(size);

		for (int i = 0; i < nrOfTimes * 2; i++) {
			if (firstPass) {
				_glowBatch.batch.renderTarget = _blurrExtraFBO2.get();
				texture->bind(1);
				firstPass = false;
			}
			else if (horizontal) {
				_glowBatch.batch.renderTarget = _blurrExtraFBO2.get();
				(*_blurrExtraFBO1)[0]->bind(1);
			}
			else {
				_glowBatch.batch.renderTarget = _blurrExtraFBO1.get();
				(*_blurrExtraFBO2)[0]->bind(1);
			}
			_glowBatch.pipeline->setValue(2, horizontal);
			_engine->getRenderer()->postProcessing(_glowBatch.batch);
			horizontal = !horizontal;
		}

		// Change back to normal rendertarget.
		_glowBatch.batch.renderTarget = _glowBatch.output.get();
		return _blurrExtraFBO1;
	}
}