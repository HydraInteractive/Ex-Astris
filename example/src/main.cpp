#include <hydra/engine.hpp>

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

		_vertexShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::vertex, "assets/shaders/base.vert");
		_geometryShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::geometry, "assets/shaders/base.geom");
		_fragmentShader = Renderer::GLShader::createFromSource(Renderer::PipelineStage::fragment, "assets/shaders/base.frag");

		_pipeline = Renderer::GLPipeline::create();
		_pipeline->attachStage(*_vertexShader);
		_pipeline->attachStage(*_geometryShader);
		_pipeline->attachStage(*_fragmentShader);

		_geometryFBO = Renderer::GLFramebuffer::create(_view->getSize(), 4);
		_geometryFBO->addTexture(0, TextureType::u8RGB)
			.addTexture(1, TextureType::u8RGB)
			.addTexture(2, TextureType::f32Depth)
			.finalize();

		_geometryWindow = _uiRenderer->addRenderWindow();
		_geometryWindow->enabled = true;
		_geometryWindow->title = "Geometry FBO";
		_geometryWindow->image = Renderer::GLTexture::createFromData(_view->getSize().x, _view->getSize().y, TextureType::u8RGB, nullptr);

		_depthWindow = _uiRenderer->addRenderWindow();
		_depthWindow->enabled = true;
		_depthWindow->title = "Depth FBO";
		_depthWindow->image = Renderer::GLTexture::createFromData(_view->getSize().x, _view->getSize().y, TextureType::u8RGB, nullptr);

		std::shared_ptr<IEntity> cameraEntity = _world->createEntity("Camera");
		_cc = cameraEntity->addComponent<Component::CameraComponent>(_geometryFBO.get(), glm::vec3{0, 0, -3});

		std::shared_ptr<IEntity> boxes = _world->createEntity("Boxes");
		boxes->addComponent<Component::TransformComponent>(glm::vec3(0, 0, 0));
		for (int x = 0; x < 3; x++) {
			std::shared_ptr<IEntity> xLevel = boxes->createEntity("X Level");
			xLevel->addComponent<Component::TransformComponent>(glm::vec3(x-1.5, 0, 0), glm::vec3(x*0.5 + 1, 1, 1));
			for (int y = 0; y < 3; y++) {
				std::shared_ptr<IEntity> yLevel = xLevel->createEntity("Y Level");
				yLevel->addComponent<Component::TransformComponent>(glm::vec3(0, y-1.5, 0), glm::vec3(1, y*0.5 + 1, 1));
				for (int z = 0; z < 3; z++) {
					std::shared_ptr<IEntity> zLevel = yLevel->createEntity("Z Level");
					zLevel->addComponent<Component::MeshComponent>("assets/objects/test.fbx");
					zLevel->addComponent<Component::TransformComponent>(glm::vec3(0, 0, z-1.5), glm::vec3(0.25, 0.25, z*0.125 + 0.25));
				}
			}
		}

		_geometryBatch.clearColor = glm::vec4(0, 0, 0, 1);
		_geometryBatch.clearFlags = ClearFlags::color | ClearFlags::depth;
		_geometryBatch.renderTarget = _geometryFBO.get();
		_geometryBatch.pipeline = _pipeline.get();

		_viewBatch.clearColor = glm::vec4(0, 0.05, 0.05, 1);
		_viewBatch.clearFlags = ClearFlags::color | ClearFlags::depth;
		_viewBatch.renderTarget = _view.get();
		_viewBatch.pipeline = _pipeline.get(); // TODO: Change to "null" pipeline

		BlueprintLoader::save("world.blueprint", "World Blueprint", _world);
	}

	~Engine() final {}

	void run() final {
		while (!_view->isClosed()) {
			_world->tick(TickAction::checkDead);
			_renderer->cleanup();

			_view->update(_uiRenderer.get());
			_uiRenderer->newFrame();

			_world->tick(TickAction::physics);

			_world->tick(TickAction::render);

			// Render to geometryFBO
			_geometryFBO->resize(_geometryWindow->size);

			_geometryShader->setValue(0, _cc->getViewMatrix());
			_geometryShader->setValue(1, _cc->getProjectionMatrix());
			_geometryShader->setValue(2, _cc->getPosition());

			_geometryBatch.objects.clear();

			for (DrawObject* drawObj : _renderer->activeDrawObjects())
				if (!drawObj->disable && drawObj->mesh)
					_geometryBatch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);

			_renderer->render(_geometryBatch);

			// Render to view
			_renderer->render(_viewBatch);

			// Resolve gemoetryFBO into the geometry window in the UI
			_geometryFBO->resolve(0, _geometryWindow->image);
			_geometryFBO->resolve(1, _depthWindow->image);
			_uiRenderer->render();

			_view->finalize();

			_world->tick(TickAction::network);
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
		_uiRenderer->getLog()->log(level, fmt, va);
		va_end(va);
	}

private:
	std::shared_ptr<IWorld> _world;

	std::unique_ptr<View::IView> _view;
	std::unique_ptr<Renderer::IRenderer> _renderer;
	std::unique_ptr<Renderer::IUIRenderer> _uiRenderer;
	std::unique_ptr<IO::TextureLoader> _textureLoader;
	std::unique_ptr<IO::MeshLoader> _meshLoader;

	std::unique_ptr<Renderer::IShader> _vertexShader;
	std::unique_ptr<Renderer::IShader> _geometryShader;
	std::unique_ptr<Renderer::IShader> _fragmentShader;
	std::unique_ptr<Renderer::IPipeline> _pipeline;

	std::shared_ptr<Renderer::IFramebuffer> _geometryFBO;
	Renderer::UIRenderWindow* _geometryWindow;
	Renderer::UIRenderWindow* _depthWindow;

	Component::CameraComponent* _cc = nullptr;
	Renderer::Batch _geometryBatch;
	Renderer::Batch _viewBatch;
};

int main(int argc, const char** argv) {
	Engine().run();
	return 0;
}
