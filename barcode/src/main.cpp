#include <hydra/engine.hpp>

#include <SDL2/SDL.h>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/glmeshloader.hpp>
#include <hydra/io/gltextureloader.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>

#include <hydra/world/blueprintloader.hpp>

#include <hydra/component/componentmanager.hpp>
#include <hydra/component/componentmanager_graphics.hpp>
#include <hydra/component/componentmanager_network.hpp>
#include <hydra/component/componentmanager_physics.hpp>
#include <hydra/component/componentmanager_sound.hpp>

#include <cstdio>

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static inline void reportMemoryLeaks() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}
#else
static inline void reportMemoryLeaks() {}
#endif

using namespace Hydra;
using namespace Hydra::World;

class Engine final : public Hydra::IEngine {
public:
	Engine() {
		Hydra::IEngine::getInstance() = this;

		_setupComponents();

		_view = View::SDLView::create();
		_renderer = Renderer::GLRenderer::create(*_view);
		_uiRenderer = Renderer::UIRenderer::create(*_view);
		_textureLoader = IO::GLTextureLoader::create();
		_meshLoader = IO::GLMeshLoader::create(_renderer.get());

		_positionWindow = _uiRenderer->addRenderWindow();
		_positionWindow->enabled = true;
		_positionWindow->title = "Position FBO";
		_positionWindow->image = Renderer::GLTexture::createFromData(_positionWindow->size.x, _positionWindow->size.y, TextureType::u8RGB, nullptr);

		_diffuseWindow = _uiRenderer->addRenderWindow();
		_diffuseWindow->enabled = true;
		_diffuseWindow->title = "Diffuse FBO";
		_diffuseWindow->image = Renderer::GLTexture::createFromData(_diffuseWindow->size.x, _diffuseWindow->size.y, TextureType::u8RGB, nullptr);

		_normalWindow = _uiRenderer->addRenderWindow();
		_normalWindow->enabled = true;
		_normalWindow->title = "Normal FBO";
		_normalWindow->image = Renderer::GLTexture::createFromData(_normalWindow->size.x, _normalWindow->size.y, TextureType::u8RGB, nullptr);

		_depthWindow = _uiRenderer->addRenderWindow();
		_depthWindow->enabled = true;
		_depthWindow->title = "Depth FBO";
		_depthWindow->image = Renderer::GLTexture::createFromData(_depthWindow->size.x, _depthWindow->size.y, TextureType::u8RGB, nullptr);

		_postTestWindow = _uiRenderer->addRenderWindow();
		_postTestWindow->enabled = true;
		_postTestWindow->title = "PostTest FBO";
		_postTestWindow->image = Renderer::GLTexture::createFromData(_postTestWindow->size.x, _postTestWindow->size.y, TextureType::u8RGB, nullptr);

		_glowWindow = _uiRenderer->addRenderWindow();
		_glowWindow->enabled = true;
		_glowWindow->title = "Glow FBO";
		_glowWindow->image = Renderer::GLTexture::createFromData(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB, nullptr);

		{
			auto& batch = _geometryBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/geometry.vert");
			batch.geometryShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::geometry, "assets/shaders/geometry.geom");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/geometry.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.geometryShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Renderer::GLFramebuffer::create(_positionWindow->size, 4);
			batch.output
				->addTexture(0, TextureType::f32RGB) // Position
				.addTexture(1, TextureType::u8RGB) // Diffuse
				.addTexture(2, TextureType::u8RGB) // Normal
				.addTexture(3, TextureType::f32RGB) // Depth
				.addTexture(4, TextureType::f32Depth) // Real Depth
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{ // Lighting pass batch
			auto& batch = _lightingBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/lighting.vert");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/lighting.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			batch.output
				->addTexture(0, TextureType::u8RGB)
				.addTexture(1, TextureType::u8RGB)
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get(); // TODO: Change to "null" pipeline
		}


		{
			auto& batch = _glowBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/blur.vert");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/blur.frag");

			_glowVertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/glow.vert");
			_glowFragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/glow.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			_glowPipeline = Renderer::GLPipeline::create();
			_glowPipeline->attachStage(*_glowVertexShader);
			_glowPipeline->attachStage(*_glowFragmentShader);
			_glowPipeline->finalize();

			batch.output = Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			batch.output
				->addTexture(0, TextureType::u8RGB)
				.finalize();

			// Extra buffer for ping-ponging the texture for two-pass gaussian blur.
			_blurrExtraFBO1 = Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			_blurrExtraFBO1
				->addTexture(0, TextureType::u8RGB)
				.finalize();
			_blurrExtraFBO2 = Renderer::GLFramebuffer::create(_glowWindow->size, 0);
			_blurrExtraFBO2
				->addTexture(0, TextureType::u8RGB)
				.finalize();

			// 3 Blurred Textures and one original.
			_blurredOriginal = Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG1 = Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG2 = Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);
			_blurredIMG3 = Renderer::GLTexture::createEmpty(_glowWindow->size.x, _glowWindow->size.y, TextureType::u8RGB);

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		{
			auto& batch = _viewBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/view.vert");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/view.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0, 0.0, 0.0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = _view.get();
			batch.batch.pipeline = batch.pipeline.get(); // TODO: Change to "null" pipeline
		}

		{
			auto& batch = _postTestBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/postTest.vert");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/postTest.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.output = Renderer::GLFramebuffer::create(_positionWindow->size, 4);
			batch.output
				->addTexture(0, TextureType::u8RGB) // Position
				.finalize();

			batch.batch.clearColor = glm::vec4(0, 0, 0, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = batch.output.get();
			batch.batch.pipeline = batch.pipeline.get();
		}

		_initEntities();
	}

	~Engine() final { }

	void run() final {
		while (!_view->isClosed()) {
			{ // Remove old dead objects
				_world->tick(TickAction::checkDead);
				_renderer->cleanup();
			}

			{ // Fetch new events
				_view->update(_uiRenderer.get());
				_uiRenderer->newFrame();
			}

			{ // Update physics
				_world->tick(TickAction::physics);
			}

			{ // Render objects (Deferred rendering)
				glm::vec3 cameraPos;
				_world->tick(TickAction::render);

				// Render to geometryFBO
				if (!_uiRenderer->isDraging()) {
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

				//_geometryBatch.batch.objects.clear();
				for (auto& kv : _geometryBatch.batch.objects)
					kv.second.clear();

				for (auto& drawObj : _renderer->activeDrawObjects())
					if (!drawObj->disable && drawObj->mesh)
						_geometryBatch.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

				// Sort Front to back
				for (auto& kv : _geometryBatch.batch.objects) {
					std::vector<glm::mat4>& list = kv.second;

					std::sort(list.begin(), list.end(), [cameraPos](const glm::mat4& a, const glm::mat4& b) {
							return glm::distance(glm::vec3(a[3]), cameraPos) < glm::distance(glm::vec3(b[3]), cameraPos);
					});
				}

				_renderer->render(_geometryBatch.batch);
			}

			{ // Lighting pass
				if (!_uiRenderer->isDraging()) {
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

				_renderer->postProcessing(_lightingBatch.batch);
			}


			{ // Glow
				if (!_uiRenderer->isDraging()) {
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

				_renderer->postProcessing(_glowBatch.batch);
				_glowBatch.batch.pipeline = _glowBatch.pipeline.get();
			}

			{ // Render transparent objects	(Forward rendering)
				_world->tick(TickAction::renderTransparent);

			}

			{
				if (!_uiRenderer->isDraging()) {
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
				_renderer->postProcessing(_postTestBatch.batch);
			}

			{ // Update UI & views
				// If you wanna see the final image, uncomment the two rows below.
				//_viewBatch.batch.pipeline->setValue(0, 0);
				//(*_glowBatch.output)[0]->bind(0);

				// Render to view
				_renderer->render(_viewBatch.batch);

				// Resolve geometryFBO into the geometry window in the UI
				_geometryBatch.output->resolve(0, _positionWindow->image);
				_geometryBatch.output->resolve(1, _diffuseWindow->image);
				_geometryBatch.output->resolve(2, _normalWindow->image);
				_geometryBatch.output->resolve(3, _depthWindow->image);
				_postTestBatch.output->resolve(0, _postTestWindow->image);
				_glowBatch.output->resolve(0, _glowWindow->image);
				//_lightingBatch.output->resolve(0, _glowWindow->image);
				//_lightingBatch.output->resolve(1, _glowWindow->image);
				_uiRenderer->render();

				_view->finalize();
			}

			{ // Sync with network
				_world->tick(TickAction::network);
			}
		}
	}


	IWorld* getWorld() final { return _world.get(); }
	Renderer::IRenderer* getRenderer() final { return _renderer.get(); }
	IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
	IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }

	void log(LogLevel level, const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		vfprintf(stderr, fmt, va);
		fputc('\n', stderr);
		va_end(va);

		va_start(va, fmt);
		_uiRenderer->getLog()->log(level, fmt, va);
		va_end(va);
	}

private:
	struct RenderBatch final {
		std::unique_ptr<Renderer::IShader> vertexShader;
		std::unique_ptr<Renderer::IShader> geometryShader;
		std::unique_ptr<Renderer::IShader> fragmentShader;
		std::unique_ptr<Renderer::IPipeline> pipeline;

		std::shared_ptr<Renderer::IFramebuffer> output;
		Renderer::Batch batch;
	};

	std::unique_ptr<View::IView> _view;
	std::unique_ptr<Renderer::IRenderer> _renderer;
	std::unique_ptr<Renderer::IUIRenderer> _uiRenderer;
	std::unique_ptr<IO::ITextureLoader> _textureLoader;
	std::unique_ptr<IO::IMeshLoader> _meshLoader;

	std::unique_ptr<IWorld> _world;

	Renderer::UIRenderWindow* _positionWindow;
	Renderer::UIRenderWindow* _diffuseWindow;
	Renderer::UIRenderWindow* _normalWindow;
	Renderer::UIRenderWindow* _glowWindow;
	Renderer::UIRenderWindow* _depthWindow;
	Renderer::UIRenderWindow* _postTestWindow;

	RenderBatch _geometryBatch; // First part of deferred rendering
	RenderBatch _lightingBatch; // Second part of deferred rendering
	RenderBatch _glowBatch; // Glow batch.
	RenderBatch _viewBatch;
	RenderBatch _postTestBatch;

	// Extra framebuffers, pipeline and shaders for glow/bloom/blur
	std::shared_ptr<Renderer::IFramebuffer> _blurrExtraFBO1;
	std::shared_ptr<Renderer::IFramebuffer> _blurrExtraFBO2;
	std::shared_ptr<Renderer::ITexture> _blurredOriginal;
	std::shared_ptr<Renderer::ITexture> _blurredIMG1;
	std::shared_ptr<Renderer::ITexture> _blurredIMG2;
	std::shared_ptr<Renderer::ITexture> _blurredIMG3;

	std::shared_ptr<Renderer::IPipeline> _glowPipeline;
	std::unique_ptr<Renderer::IShader> _glowVertexShader;
	std::unique_ptr<Renderer::IShader> _glowFragmentShader;

	Component::CameraComponent* _cc = nullptr;

	void _setupComponents() {
		using namespace Hydra::Component::ComponentManager;
		auto& map = createOrGetComponentMap();
		registerComponents_graphics(map);
		registerComponents_network(map);
		registerComponents_physics(map);
		registerComponents_sound(map);
	}

	void _initEntities() {
		_world = Hydra::World::World::create();
		auto cameraEntity = _world->createEntity("Camera");
		/*_cc = */ cameraEntity->addComponent<Component::CameraComponent>(_geometryBatch.output.get(), glm::vec3{0, 0, -3});

		auto boxes = _world->createEntity("Boxes");
		boxes->addComponent<Component::TransformComponent>(glm::vec3(0, 0, 0));
		for (int x = 0; x < 3; x++) {
			auto xLevel = boxes->createEntity("X Level");
			xLevel->addComponent<Component::TransformComponent>(glm::vec3(x-1.5, 0, 0), glm::vec3(x*0.5 + 1, 1, 1));
			for (int y = 0; y < 3; y++) {
				auto yLevel = xLevel->createEntity("Y Level");
				yLevel->addComponent<Component::TransformComponent>(glm::vec3(0, y-1.5, 0), glm::vec3(1, y*0.5 + 1, 1));
				for (int z = 0; z < 3; z++) {
					auto zLevel = yLevel->createEntity("Z Level");
					zLevel->addComponent<Component::MeshComponent>("assets/objects/test.fbx");
					zLevel->addComponent<Component::TransformComponent>(glm::vec3(0, 0, z-1.5), glm::vec3(0.25, 0.25, z*0.125 + 0.25));
				}
			}
		}

		BlueprintLoader::save("world.blueprint", "World Blueprint", _world->getWorldRoot());
		auto bp = BlueprintLoader::load("world.blueprint");
		_world->setWorldRoot(bp->spawn(_world.get()));

		{
			auto& world = _world->getWorldRoot()->getChildren();
			auto it = std::find_if(world.begin(), world.end(), [](const std::shared_ptr<IEntity>& e) { return e->getName() == "Camera"; });
			if (it != world.end()) {
				_cc = (*it)->getComponent<Component::CameraComponent>();
				_cc->setRenderTarget(_geometryBatch.output.get());
			} else
				log(LogLevel::error, "Camera not found!");
		}
	}

	std::shared_ptr<Renderer::IFramebuffer> _blurGlowTexture(std::shared_ptr<ITexture>& texture, int &nrOfTimes, glm::vec2 size) { // TO-DO: Make it agile so it can blur any texture

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
			_renderer->postProcessing(_glowBatch.batch);
			horizontal = !horizontal;
		}
		// Change back to normal rendertarget.
		_glowBatch.batch.renderTarget = _glowBatch.output.get();
		return _blurrExtraFBO1;
	}


};

#undef main
int main(int argc, char** argv) {
	reportMemoryLeaks();
	Engine().run();
	return 0;
}
