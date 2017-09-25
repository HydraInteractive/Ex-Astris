#include <barcode/gamestate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <hydra/world/blueprintloader.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	GameState::GameState() : _engine(Hydra::IEngine::getInstance()) {}

	void GameState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		_positionWindow = _engine->getUIRenderer()->addRenderWindow();
		_positionWindow->enabled = true;
		_positionWindow->title = "Position FBO";
		_positionWindow->image = Hydra::Renderer::GLTexture::createFromData(_positionWindow->size.x, _positionWindow->size.y, TextureType::u8RGB, nullptr);

		_diffuseWindow = _engine->getUIRenderer()->addRenderWindow();
		_diffuseWindow->enabled = true;
		_diffuseWindow->title = "Diffuse FBO";
		_diffuseWindow->image = Hydra::Renderer::GLTexture::createFromData(_diffuseWindow->size.x, _diffuseWindow->size.y, TextureType::u8RGB, nullptr);

		_normalWindow = _engine->getUIRenderer()->addRenderWindow();
		_normalWindow->enabled = true;
		_normalWindow->title = "Normal FBO";
		_normalWindow->image = Hydra::Renderer::GLTexture::createFromData(_normalWindow->size.x, _normalWindow->size.y, TextureType::u8RGB, nullptr);

		_depthWindow = _engine->getUIRenderer()->addRenderWindow();
		_depthWindow->enabled = true;
		_depthWindow->title = "Depth FBO";
		_depthWindow->image = Hydra::Renderer::GLTexture::createFromData(_depthWindow->size.x, _depthWindow->size.y, TextureType::u8RGB, nullptr);

		_postTestWindow = _engine->getUIRenderer()->addRenderWindow();
		_postTestWindow->enabled = true;
		_postTestWindow->title = "PostTest FBO";
		_postTestWindow->image = Hydra::Renderer::GLTexture::createFromData(_postTestWindow->size.x, _postTestWindow->size.y, TextureType::u8RGB, nullptr);

		_glowWindow = _engine->getUIRenderer()->addRenderWindow();
		_glowWindow->enabled = true;
		_glowWindow->title = "Glow FBO";
		_glowWindow->image = Hydra::Renderer::GLTexture::createFromData(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB, nullptr);

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

			batch.output = Hydra::Renderer::GLFramebuffer::create(_positionWindow->size, 4);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::f32RGB) // Position
				.addTexture(1, Hydra::Renderer::TextureType::u8RGB) // Diffuse
				.addTexture(2, Hydra::Renderer::TextureType::u8RGB) // Normal
				.addTexture(3, Hydra::Renderer::TextureType::f32RGB) // Depth
				.addTexture(4, Hydra::Renderer::TextureType::f32Depth) // Real Depth
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
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/lighting.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/lighting.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(_glowWindow->size, 0);
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

			batch.output = Hydra::Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();

			// Extra buffer for ping-ponging the texture for two-pass gaussian blur.
			_blurrExtraFBO1 = Hydra::Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			_blurrExtraFBO1
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();
			_blurrExtraFBO2 = Hydra::Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			_blurrExtraFBO2
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB)
				.finalize();

			// 3 Blurred Textures and one original.
			_blurredOriginal = Hydra::Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG1 = Hydra::Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG2 = Hydra::Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG3 = Hydra::Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
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

			_particleAtlases = Hydra::Renderer::GLTexture::createFromFile("assets/textures/ParticleAtlases.png");

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = _engine->getView();
			batch.batch.pipeline = batch.pipeline.get();
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
			auto& batch = _postTestBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/postTest.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/postTest.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Hydra::Renderer::GLFramebuffer::create(_positionWindow->size, 4);
			batch.output
				->addTexture(0, Hydra::Renderer::TextureType::u8RGB) // Position
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		_initWorld();
	}

	GameState::~GameState() { }

	void GameState::onMainMenu() { }

	//Global variable to maintain a keyframe for now


	void GameState::runFrame(float delta) {
		{ // Fetch new events
			_engine->getView()->update(_engine->getUIRenderer());
			_engine->getUIRenderer()->newFrame();
		}

		{ // Update physics
			_world->tick(TickAction::physics, delta);
		}

		{ // Render objects (Deferred rendering)
			glm::vec3 cameraPos;
			_world->tick(TickAction::render, delta);

			// Render to geometryFBO
			if (!_engine->getUIRenderer()->isDraging()) {
				static glm::ivec2 oldSize = _geometryBatch.output->getSize();
				auto newSize = _positionWindow->size;
				if (oldSize != newSize) {
					_geometryBatch.output->resize(newSize);
					oldSize = newSize;
				}
			}

			_geometryBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_geometryBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_geometryBatch.pipeline->setValue(2, cameraPos = _cc->getPosition());


			_animationBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_animationBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_animationBatch.pipeline->setValue(2, cameraPos = _cc->getPosition());

			//_geometryBatch.batch.objects.clear();
			for (auto& kv : _geometryBatch.batch.objects)
				kv.second.clear();

			for (auto& kv : _animationBatch.batch.objects)
				kv.second.clear();

			for (auto& drawObj : _engine->getRenderer()->activeDrawObjects()) {

				if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == false)
					_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
				else if (!drawObj->disable && drawObj->mesh && drawObj->mesh->hasAnimation() == true) {
					_animationBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

					//Get number of keyframes. Also hardcoded for now
					if (currentFrame < 75) {
						currentFrame++;
					}
					else {
						currentFrame = 1;
					}

					std::vector<glm::mat4> tempMats;
					//i == nrOfJoints. Hardcoded for now
					for (int i = 0; i < 4; i++) {
						tempMats.push_back(drawObj->mesh->getTransformationMatrices(0, i, currentFrame - 1));
						_animationBatch.pipeline->setValue(11 + i, tempMats[i]);
					}
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

			_engine->getRenderer()->render(_animationBatch.batch);
		}

		{ // Lighting pass
			if (!_engine->getUIRenderer()->isDraging()) {
				static glm::ivec2 oldSize = _lightingBatch.output->getSize();
				auto newSize = _glowWindow->size;
				if (oldSize != newSize) {
					_lightingBatch.output->resize(newSize);
					oldSize = newSize;
				}
			}

			_lightingBatch.pipeline->setValue(0, 0);
			_lightingBatch.pipeline->setValue(1, 1);
			_lightingBatch.pipeline->setValue(2, 2);
			_lightingBatch.pipeline->setValue(3, _cc->getPosition());

			(*_geometryBatch.output)[0]->bind(0);
			(*_geometryBatch.output)[1]->bind(1);
			(*_geometryBatch.output)[2]->bind(2);

			_engine->getRenderer()->postProcessing(_lightingBatch.batch);
		}

		{ // Glow
			if (!_engine->getUIRenderer()->isDraging()) {
				static glm::ivec2 oldSize = _glowBatch.output->getSize();
				auto newSize = _glowWindow->size;
				if (oldSize != newSize) {
					_glowBatch.output->resize(newSize);
					_blurrExtraFBO1->resize(newSize);
					_blurrExtraFBO2->resize(newSize);
					oldSize = newSize;
				}
			}
			// Resolves the glow texture from geomtrybatch which returns an image, that then is
			// put into the function which returns a framebuffer that is then put into position 0 in blurredTexturesFBO
			// Not sure why I can't copy textures from different framebuffers to eachother, have to look into it later.
			int nrOfTimes = 1;
			glm::vec2 size = _glowWindow->size;

			_lightingBatch.output->resolve(0, _blurredOriginal);
			_lightingBatch.output->resolve(1, (*_glowBatch.output)[0]);

			_blurGlowTexture((*_glowBatch.output)[0], nrOfTimes, size *= 0.5f)
				->resolve(0, _blurredIMG1);
			_blurGlowTexture(_blurredIMG1, nrOfTimes, size *= 0.5f)
				->resolve(0, _blurredIMG2);
			_blurGlowTexture(_blurredIMG2, nrOfTimes, size *= 0.5f)
				->resolve(0, _blurredIMG3);

			_glowBatch.batch.pipeline = _glowPipeline.get();

			_glowBatch.batch.pipeline->setValue(1, 1);
			_glowBatch.batch.pipeline->setValue(2, 2);
			_glowBatch.batch.pipeline->setValue(3, 3);
			_glowBatch.batch.pipeline->setValue(4, 4);

			_blurredOriginal->bind(1);
			_blurredIMG1->bind(2);
			_blurredIMG2->bind(3);
			_blurredIMG3->bind(4);

			_engine->getRenderer()->postProcessing(_glowBatch.batch);
			_glowBatch.batch.pipeline = _glowBatch.pipeline.get();
		}

		{ // Render transparent objects	(Forward rendering)
			_world->tick(TickAction::renderTransparent, delta);
		}

		{ // Particle batch
			for (auto& kv : _particleBatch.batch.objects)
				kv.second.clear();

			for (auto entity : _world->getActiveComponents<Hydra::Component::ParticleComponent>()) {
				auto pc = entity->getComponent<Hydra::Component::ParticleComponent>();
				auto drawObj = entity->getDrawObject();
				for (auto particle : pc->getParticles()) {
					_particleBatch.batch.objects[drawObj->mesh].push_back(particle->m);
				}
			}
			_engine->getRenderer()->render(_particleBatch.batch);
		}

		{
			if (!_engine->getUIRenderer()->isDraging()) {
				static glm::ivec2 oldSize = _postTestBatch.output->getSize();
				auto newSize = _positionWindow->size;
				if (oldSize != newSize) {
					_postTestBatch.output->resize(newSize);
					oldSize = newSize;
				}
			}
			(*_geometryBatch.output)[0]->bind(0);
			_postTestBatch.pipeline->setValue(0, 0);
			_postTestBatch.pipeline->setValue(1, (int)_geometryBatch.output->getSamples());
			_engine->getRenderer()->postProcessing(_postTestBatch.batch);
		}

		{ // Hud windows
			static float f = 0.0f;
			static bool b = false;
			static float invisF[3] = { 0, 0, 0 };
			float hpP = 100;
			float ammoP = 100;
			
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, float(0.0f));

			const ImVec2 pos = ImVec2(_engine->getView()->getSize().x / 2, _engine->getView()->getSize().y / 2);

			ImGui::SetNextWindowPos(pos + ImVec2(-10, 1));
			ImGui::SetNextWindowSize(ImVec2(20, 20));
			ImGui::Begin("Crosshair", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Crosshair.png")->getID()), ImVec2(20, 20));
			ImGui::End();

			ImGui::SetNextWindowPos(pos + ImVec2(-51, -42));
			ImGui::SetNextWindowSize(ImVec2(120, 120));
			ImGui::Begin("AimRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AimRing.png")->getID()), ImVec2(100, 100));
			ImGui::End();

			float offsetHpF = 72 * hpP * 0.01;
			int offsetHp = offsetHpF;
			ImGui::SetNextWindowPos(pos + ImVec2(-47, -26 + 72 - offsetHp));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("HpOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/HpOnRing.png")->getID()), ImVec2(22, offsetHp), ImVec2(0, 1 - hpP * 0.01), ImVec2(1, 1));
			ImGui::End();

			float offsetAmmoF = 72 * ammoP * 0.01;
			int offsetAmmo = offsetAmmoF;
			ImGui::SetNextWindowPos(pos + ImVec2(+25, -26 + 72 - offsetAmmo));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("AmmoOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AmmoOnRing.png")->getID()), ImVec2(22, offsetAmmo), ImVec2(0, 1 - ammoP * 0.01), ImVec2(1, 1));
			ImGui::End();

			float degres = 0;
			float degresP = ((float(100) / float(360) * degres)/100);
			float degresO = float(1000) * degresP;
			ImGui::SetNextWindowPos(ImVec2(pos.x - 275, + 70));
			ImGui::SetNextWindowSize(ImVec2(600, 20));
			ImGui::Begin("Compass", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Compass.png")->getID()), ImVec2(550, 20), ImVec2(degresO / float(1550), 0), ImVec2((float(1) - ((float(1000) - degresO) / float(1550))), 1));
			_textureLoader->getTexture("assets/hud/Compass.png")->setRepeat();
			ImGui::End();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

		}

		{ // Update UI & views
			// If you wanna see the final image, uncomment the two rows below.
			//_viewBatch.batch.pipeline->setValue(0, 0);
			//(*_glowBatch.output)[0]->bind(0);

			// Render to view
			//_engine->getRenderer()->render(_viewBatch.batch);

			// Resolve geometryFBO into the geometry window in the UI
			_geometryBatch.output->resolve(0, _positionWindow->image);
			_geometryBatch.output->resolve(1, _diffuseWindow->image);
			_geometryBatch.output->resolve(2, _normalWindow->image);
			_geometryBatch.output->resolve(3, _depthWindow->image);
			_postTestBatch.output->resolve(0, _postTestWindow->image);
			_glowBatch.output->resolve(0, _glowWindow->image);
			//_lightingBatch.output->resolve(0, _glowWindow->image);
			//_lightingBatch.output->resolve(1, _glowWindow->image);
		}

		{ // Sync with network
			_world->tick(TickAction::network, delta);
		}
	}

	void GameState::_initWorld() {
		_world = Hydra::World::World::create();


		auto playerEntity = _world->createEntity("Player");
		player = playerEntity->addComponent<Hydra::Component::PlayerComponent>();
		_cc = playerEntity->addComponent<Hydra::Component::CameraComponent>(_geometryBatch.output.get(), glm::vec3{ 5, 0, -3 });
		playerEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(0, 0, 0));

		auto animatedEntity = _world->createEntity("AnimatedCube");
		animatedEntity->addComponent<Hydra::Component::MeshComponent>("assets/objects/animatedCube.ATTIC");
		animatedEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(-10, 0, -10));


		auto weaponEntity = playerEntity->createEntity("Weapon");
		weaponEntity->addComponent<Hydra::Component::MeshComponent>("assets/objects/alphaGunModel.ATTIC");
		weaponEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(0, 0, 0), glm::vec3(1,1,1), glm::quat(0,0,-1,0));
		
		auto particleEmitter = _world->createEntity("ParticleEmitter");
		particleEmitter->addComponent<Hydra::Component::ParticleComponent>(Hydra::Component::EmitterBehaviour::PerSecond, 1);

		auto alienEntity = _world->createEntity("Enemy Alien");
		_enemy = alienEntity->addComponent<Hydra::Component::EnemyComponent>(Hydra::Component::EnemyTypes::Alien);
		alienEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(-10, 0, 0));
		alienEntity->addComponent<Hydra::Component::MeshComponent>("assets/objects/alphaGunModel.ATTIC");

		auto robotEntity = _world->createEntity("Enemy Robot");
		_enemy = robotEntity->addComponent<Hydra::Component::EnemyComponent>(Hydra::Component::EnemyTypes::Robot);
		robotEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(15, 0, 0));
		robotEntity->addComponent<Hydra::Component::MeshComponent>("assets/objects/alphaGunModel.ATTIC");

		auto bossEntity = _world->createEntity("Enemy Boss");
		_enemy = bossEntity->addComponent<Hydra::Component::EnemyComponent>(Hydra::Component::EnemyTypes::AlienBoss);
		bossEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(0, -10, 0));
		bossEntity->addComponent<Hydra::Component::MeshComponent>("assets/objects/alphaGunModel.ATTIC");

		BlueprintLoader::save("world.blueprint", "World Blueprint", _world->getWorldRoot());
		auto bp = BlueprintLoader::load("world.blueprint");
		_world->setWorldRoot(bp->spawn(_world.get()));

		{
			auto& world = _world->getWorldRoot()->getChildren();
			auto it = std::find_if(world.begin(), world.end(), [](const std::shared_ptr<IEntity>& e) { return e->getName() == "Player"; });
			if (it != world.end()) {
				_cc = (*it)->getComponent<Hydra::Component::CameraComponent>();
				_cc->setRenderTarget(_geometryBatch.output.get());
			} else
				_engine->log(Hydra::LogLevel::error, "Camera not found!");
		}

	}

	std::shared_ptr<Hydra::Renderer::IFramebuffer> GameState::_blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int &nrOfTimes, glm::vec2 size) { // TO-DO: Make it agile so it can blur any texture
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
