#include <hydra/engine.hpp>

#include <SDL2/SDL.h>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/renderer/uirenderer.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>

#include <hydra/world/blueprintloader.hpp>

#include <cstdio>

using namespace Hydra;
using namespace Hydra::World;

class Engine final : public Hydra::IEngine {
public:
	Engine() {
		Hydra::IEngine::getInstance() = this;

		_world = Hydra::World::World::create();

		_view = View::SDLView::create();
		_renderer = Renderer::GLRenderer::create(*_view);
		_uiRenderer = Renderer::UIRenderer::create(*_view);
		_textureLoader = std::make_unique<IO::TextureLoader>();
		_meshLoader = std::make_unique<IO::MeshLoader>(_renderer.get());

		_geometryWindow = _uiRenderer->addRenderWindow();
		_geometryWindow->enabled = true;
		_geometryWindow->title = "Geometry FBO";
		_geometryWindow->image = Renderer::GLTexture::createFromData(_geometryWindow->size.x, _geometryWindow->size.y, TextureType::u8RGB, nullptr);

		_depthWindow = _uiRenderer->addRenderWindow();
		_depthWindow->enabled = true;
		_depthWindow->title = "Depth FBO";
		_depthWindow->image = Renderer::GLTexture::createFromData(_geometryWindow->size.x, _geometryWindow->size.y, TextureType::u8RGB, nullptr);

		{
			auto& batch = _geometryBatch;
			batch.vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/geometry.vert");
			batch.geometryShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::geometry, "assets/shaders/geometry.geom");
			batch.fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/geometry.frag");

			batch.pipeline = Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.geometryShader);
			batch.pipeline->attachStage(*batch.fragmentShader);

			batch.output = Renderer::GLFramebuffer::create(_geometryWindow->size, 0);
			batch.output->addTexture(0, TextureType::u8RGB)
				.addTexture(1, TextureType::u8RGB)
				.addTexture(2, TextureType::f32Depth)
				.finalize();

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

			batch.batch.clearColor = glm::vec4(0, 0.05, 0.05, 1);
			batch.batch.clearFlags = ClearFlags::color | ClearFlags::depth;
			batch.batch.renderTarget = _view.get();
			batch.batch.pipeline = batch.pipeline.get(); // TODO: Change to "null" pipeline
		}

		_initEntities();
	}

	~Engine() final {}

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
				_geometryBatch.output->resize(_geometryWindow->size);

				_geometryBatch.geometryShader->setValue(0, _cc->getViewMatrix());
				_geometryBatch.geometryShader->setValue(1, _cc->getProjectionMatrix());
				_geometryBatch.geometryShader->setValue(2, cameraPos = _cc->getPosition());

				_geometryBatch.batch.objects.clear();

				for (DrawObject* drawObj : _renderer->activeDrawObjects())
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

			{ // Render transparent objects	(Forward rendering)
				_world->tick(TickAction::renderTransparent);
			}

			{ // Update UI & views
				// Render to view
				_renderer->render(_viewBatch.batch);

				// Resolve gemoetryFBO into the geometry window in the UI
				_geometryBatch.output->resolve(0, _geometryWindow->image);
				_geometryBatch.output->resolve(1, _depthWindow->image);
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
	IO::TextureLoader* getTextureLoader() final { return _textureLoader.get(); }
	IO::MeshLoader* getMeshLoader() final { return _meshLoader.get(); }

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
	std::shared_ptr<IWorld> _world;

	std::unique_ptr<View::IView> _view;
	std::unique_ptr<Renderer::IRenderer> _renderer;
	std::unique_ptr<Renderer::IUIRenderer> _uiRenderer;
	std::unique_ptr<IO::TextureLoader> _textureLoader;
	std::unique_ptr<IO::MeshLoader> _meshLoader;

	Renderer::UIRenderWindow* _geometryWindow;
	Renderer::UIRenderWindow* _depthWindow;

	RenderBatch _geometryBatch; // First part of deferred rendering
	RenderBatch _lightingBatch; // Second part of deferred rendering
	RenderBatch _viewBatch;

	Component::CameraComponent* _cc = nullptr;

	void _initEntities() {
		auto cameraEntity = _world->createEntity("Camera");
		_cc = cameraEntity->addComponent<Component::CameraComponent>(_geometryBatch.output.get(), glm::vec3{0, 0, -3});

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

		BlueprintLoader::save("world.blueprint", "World Blueprint", _world);
	}
};

#undef main
int main(int argc, char** argv) {
	Engine().run();
	return 0;
}
