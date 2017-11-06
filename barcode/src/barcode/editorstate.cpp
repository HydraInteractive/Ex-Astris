#include <barcode/editorstate.hpp>

#include <hydra/component/rigidbodycomponent.hpp>
using world = Hydra::World::World;
namespace Barcode {
	EditorState::EditorState() : _engine(Hydra::IEngine::getInstance()) {}

	void EditorState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		_finalImage = _engine->getUIRenderer()->addRenderWindow();
		_finalImage->enabled = true;
		_finalImage->title = "Final Image";
		_finalImage->image = Hydra::Renderer::GLTexture::createFromData(_finalImage->size.x, _finalImage->size.y, TextureType::u8RGB, nullptr);
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
			batch.pipeline->finalize();;

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::none;
			batch.batch.renderTarget = _geometryBatch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{ // Lighting pass batch
			auto& batch = _lightingBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/editorLighting.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/editorLighting.frag");

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

			// Extra buffer for ping-ponging the texture for two-pass gaussian blur.
			_blurrExtraFBO1 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			_blurrExtraFBO1
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();
			_blurrExtraFBO2 = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
			_blurrExtraFBO2
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();

			_fiveGaussianKernel1 = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };
			_fiveGaussianKernel2 = { 0.102637f, 0.238998f, 0.31673f, 0.238998f, 0.102637f };

			// 3 Blurred Textures and one original.
			_blurredOriginal = Hydra::Renderer::GLTexture::createEmpty(windowSize.x, windowSize.y, TextureType::u8RGB);
			_blurredIMG1 = Hydra::Renderer::GLTexture::createEmpty(windowSize.x, windowSize.y, TextureType::u8RGB);
			_blurredIMG2 = Hydra::Renderer::GLTexture::createEmpty(windowSize.x, windowSize.y, TextureType::u8RGB);
			_blurredIMG3 = Hydra::Renderer::GLTexture::createEmpty(windowSize.x, windowSize.y, TextureType::u8RGB);

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{ // PARTICLES
			auto& batch = _particleBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/particles.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/particles.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			_particleAtlases = Hydra::Renderer::GLTexture::createFromFile("assets/textures/TempAtlas.png");

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::depth;
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
		}

		{ // SSAO
			auto& batch = _ssaoBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/ssao.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/ssao.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(windowSize, 0);
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
				scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
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
			auto& batch = _previewBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/previewWindow.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/previewWindow.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(glm::vec2(720), 0);
			batch.output->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		_initWorld();
		this->_importerMenu = new ImporterMenu();
		this->_exporterMenu = new ExporterMenu();
	}

	EditorState::~EditorState() { }

	void EditorState::onMainMenu() {
		if (ImGui::BeginMenu("Editor"))
		{
			if (ImGui::MenuItem("Import..."))
			{
				_showImporter = !_showImporter;
				if (_showImporter)
					_importerMenu->refresh("/assets");
			}
			if (ImGui::MenuItem("Export..."))
			{
				_showExporter = !_showExporter;
				if (_showExporter)
					_exporterMenu->refresh("/assets");
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Clear room"))
			{
				if(FileManager::getRoomEntity() != nullptr)
					FileManager::getRoomEntity()->dead = true;

				auto room = world::newEntity("Room", world::root());
				auto t = room->addComponent<Hydra::Component::TransformComponent>();
				auto r = room->addComponent<Hydra::Component::RoomComponent>();

			}
			ImGui::EndMenu();
		}
	}

	//Global variable to maintain a keyframe for now
	void EditorState::runFrame(float delta) {
		auto windowSize = _engine->getView()->getSize();
		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		_playerSystem.tick(delta);
		_abilitySystem.tick(delta);
		_lightSystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);

		const glm::vec3 cameraPos = _cc->position;

		{ // Render objects (Deferred rendering)
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

			for (auto& kv : _geometryBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _animationBatch.batch.objects)
				kv.second.clear();

			for (auto& drawObj : _engine->getRenderer()->activeDrawObjects()) {
				if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == false)
					_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

				else if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == true) {
					_animationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

					//int currentFrame = drawObj->mesh->getCurrentKeyframe();

					//if (currentFrame < drawObj->mesh->getMaxFramesForAnimation()) {
					//	drawObj->mesh->setCurrentKeyframe(currentFrame + 1);
					//}
					//else {
					//	drawObj->mesh->setCurrentKeyframe(1);
					//}

					//glm::mat4 tempMat;
					//for (int i = 0; i < drawObj->mesh->getNrOfJoints(); i++) {
					//	tempMat = drawObj->mesh->getTransformationMatrices(i);
					//	_animationBatch.pipeline->setValue(11 + i, tempMat);
					//}
				}
			}

			// Sort Front to back
			for (auto& kv : _geometryBatch.batch.objects) {
				std::vector<glm::mat4>& list = kv.second;

				std::sort(list.begin(), list.end(), [cameraPos](const glm::mat4& a, const glm::mat4& b) {
					return glm::distance(glm::vec3(a[3]), cameraPos) < glm::distance(glm::vec3(b[3]), cameraPos);
				});
			}
			// Sort Front to back for animation
			for (auto& kv : _animationBatch.batch.objects) {
				std::vector<glm::mat4>& list = kv.second;

				std::sort(list.begin(), list.end(), [cameraPos](const glm::mat4& a, const glm::mat4& b) {
					return glm::distance(glm::vec3(a[3]), cameraPos) < glm::distance(glm::vec3(b[3]), cameraPos);
				});
			}

			_engine->getRenderer()->render(_geometryBatch.batch);
			_engine->getRenderer()->renderAnimation(_animationBatch.batch);
			//_engine->getRenderer()->render(_shadowBatch.batch);
			//_engine->getRenderer()->renderAnimation(_animationBatch.batch);
		}

		{
			for (auto& kv : _shadowBatch.batch.objects)
				kv.second.clear();

			for (auto& drawObj : _engine->getRenderer()->activeDrawObjects()) {
				if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == false)
					_shadowBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
			}

			_shadowBatch.pipeline->setValue(0, _light->getViewMatrix());
			_shadowBatch.pipeline->setValue(1, _light->getProjectionMatrix());

			_engine->getRenderer()->render(_shadowBatch.batch);
		}

		{
			for (auto& kv : _shadowBatch.batch.objects)
				kv.second.clear();

			for (auto& drawObj : _engine->getRenderer()->activeDrawObjects())
				if (!drawObj->disable && drawObj->mesh)
					_shadowBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

			_shadowBatch.pipeline->setValue(0, _light->getViewMatrix());
			_shadowBatch.pipeline->setValue(1, _light->getProjectionMatrix());

			//_engine->getRenderer()->render(_shadowBatch.batch);
			_engine->getRenderer()->renderShadows(_shadowBatch.batch);
		}

		static bool enableSSAO = true;
		ImGui::Checkbox("Enable SSAO", &enableSSAO);

		{

			_ssaoBatch.pipeline->setValue(0, 0);
			_ssaoBatch.pipeline->setValue(1, 1);
			_ssaoBatch.pipeline->setValue(2, 2);


			_ssaoBatch.pipeline->setValue(3, _cc->getProjectionMatrix());

			(*_geometryBatch.output)[4]->bind(0);
			(*_geometryBatch.output)[2]->bind(1);
			_ssaoNoise->bind(2);

			_engine->getRenderer()->postProcessing(_ssaoBatch.batch);
			int nrOfTImes = 1;
			_blurGlowTexture((*_ssaoBatch.output)[0], nrOfTImes, (*_ssaoBatch.output)[0]->getSize(), _fiveGaussianKernel1)
				->resolve(0, (*_ssaoBatch.output)[0]);
		}

		{ // Lighting pass
			_lightingBatch.pipeline->setValue(0, 0);
			_lightingBatch.pipeline->setValue(1, 1);
			_lightingBatch.pipeline->setValue(2, 2);
			_lightingBatch.pipeline->setValue(3, 3);
			_lightingBatch.pipeline->setValue(4, 4);
			_lightingBatch.pipeline->setValue(5, 5);
			_lightingBatch.pipeline->setValue(6, 6);

			_lightingBatch.pipeline->setValue(7, _cc->position);
			_lightingBatch.pipeline->setValue(8, enableSSAO);
			auto& lights = Hydra::Component::PointLightComponent::componentHandler->getActiveComponents();

			_lightingBatch.pipeline->setValue(9, (int)(lights.size()));
			_lightingBatch.pipeline->setValue(10, _light->direction);
			_lightingBatch.pipeline->setValue(11, _light->color);

			// good code lmao XD
			int i = 12;
			for (auto& p : lights) {
				auto pc = static_cast<Hydra::Component::PointLightComponent*>(p.get());
				_lightingBatch.pipeline->setValue(i++, pc->position);
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

		{ // Glow
			int nrOfTimes = 1;
			glm::vec2 size = windowSize;

			_lightingBatch.output->resolve(0, _blurredOriginal);
			_lightingBatch.output->resolve(1, (*_glowBatch.output)[0]);

			_blurGlowTexture((*_glowBatch.output)[0], nrOfTimes, size * 0.25f, _fiveGaussianKernel2)
				->resolve(0, _blurredIMG1);
			_blurGlowTexture(_blurredIMG1, nrOfTimes + 1, size * 0.25f, _fiveGaussianKernel2)
				->resolve(0, _blurredIMG2);

			_glowBatch.batch.pipeline = _glowPipeline.get();

			_glowBatch.batch.pipeline->setValue(1, 1);
			_glowBatch.batch.pipeline->setValue(2, 2);
			_glowBatch.batch.pipeline->setValue(3, 3);

			_blurredOriginal->bind(1);
			_blurredIMG1->bind(2);
			_blurredIMG2->bind(3);

			_engine->getRenderer()->postProcessing(_glowBatch.batch);
			_glowBatch.batch.pipeline = _glowBatch.pipeline.get();
		}

		{ // Particle batch
			for (auto& kv : _particleBatch.batch.objects) {
				kv.second.clear();
				_particleBatch.batch.textureInfo.clear();
			}

			for (auto& pc : Hydra::Component::ParticleComponent::componentHandler->getActiveComponents()) {
				auto p = static_cast<Hydra::Component::ParticleComponent*>(pc.get());
				auto e = world::getEntity(p->entityID);
				auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>();
				auto t = e->getComponent<Hydra::Component::TransformComponent>();
				auto& particles = p->particles;
				for (auto& particle : particles) {
					if (particle.life <= 0)
						continue;
					_particleBatch.batch.objects[drawObj->drawObject->mesh].push_back(/*t->getMatrix() */ particle.getMatrix());
					_particleBatch.batch.textureInfo.push_back(particle.texOffset1);
					_particleBatch.batch.textureInfo.push_back(particle.texOffset2);
					_particleBatch.batch.textureInfo.push_back(particle.texCoordInfo);
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

				for (auto& kv : _particleBatch.batch.objects) {
					std::vector<glm::mat4>& list = kv.second;

					std::sort(list.begin(), list.end(), [cameraPos](const glm::mat4& a, const glm::mat4& b) {
						return glm::distance(glm::vec3(a[3]), cameraPos) < glm::distance(glm::vec3(b[3]), cameraPos);
					});
				}

				_engine->getRenderer()->render(_particleBatch.batch);
			}
		}
		if (_showImporter)
			_importerMenu->render(_showImporter, &_previewBatch.batch, delta);
		if (_showExporter)
			_exporterMenu->render(_showExporter);
		_glowBatch.output->resolve(0, _finalImage->image);
		_glowBatch.batch.renderTarget = _engine->getView();
		_engine->getRenderer()->clear(_glowBatch.batch);
		_glowBatch.batch.renderTarget = _glowBatch.output.get();
	}
	void EditorState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_lightSystem, &_particleSystem, &_abilitySystem, &_aiSystem, &_physicsSystem, &_bulletSystem, &_playerSystem, &_rendererSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}
	void EditorState::_initWorld() {
		
		{
			auto room = world::newEntity("Room", world::root());
			auto t = room->addComponent<Hydra::Component::TransformComponent>();
			auto r = room->addComponent<Hydra::Component::RoomComponent>();
		}
		{
			auto playerEntity = world::newEntity("Player", world::root());
			auto c = playerEntity->addComponent<Hydra::Component::FreeCameraComponent>();
			auto t = playerEntity->addComponent<Hydra::Component::TransformComponent>();
		}
		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
			l->position = glm::vec3(-5, 0.75, 4.3);
			l->direction = glm::vec3(-1, 0, 0);
			auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(8.0, 0, 3.5);
		}
		{
			BlueprintLoader::save("world.blueprint", "World Blueprint", world::root());
			Hydra::World::World::reset();
			auto bp = BlueprintLoader::load("world.blueprint");
			bp->spawn(world::root());
		}

		{
			_cc = static_cast<Hydra::Component::FreeCameraComponent*>(Hydra::Component::FreeCameraComponent::componentHandler->getActiveComponents()[0].get());
			for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents()) {
				_engine->log(Hydra::LogLevel::normal, "Enabling bullet for %s", world::getEntity(rb->entityID)->name.c_str());
				_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
			}
		}
	}

	std::shared_ptr<Hydra::Renderer::IFramebuffer> EditorState::_blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int nrOfTimes, glm::vec2 size, const std::vector<float>& kernel) { // TO-DO: Make it agile so it can blur any texture
		_glowBatch.pipeline->setValue(1, 1); // This bind will never change
		bool horizontal = true;
		bool firstPass = true;
		_blurrExtraFBO1->resize(size);
		_blurrExtraFBO2->resize(size);
		_glowBatch.pipeline->setValue(3, 5);
		for (int i = 0; i < 5; i++) {
			_glowBatch.pipeline->setValue(4 + i, kernel[i]);
		}
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
