#include <hydra/engine.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>

#include <hydra/component/meshcomponent.hpp>

#include <hydra/world/blueprintloader.hpp>

namespace HW = Hydra::World;
namespace HV = Hydra::View;
namespace HR = Hydra::Renderer;
namespace HC = Hydra::Component;

class Engine final : public Hydra::IEngine {
public:
	Engine() {
		_world = HW::World::create();

		_view = HV::SDLView::create();
		_renderer = HR::GLRenderer::create(*_view);

		_vertexShader = HR::GLShader::createFromSource(HR::PipelineStage::vertex, "assets/shaders/base.vert");
		_fragmentShader = HR::GLShader::createFromSource(HR::PipelineStage::fragment, "assets/shaders/base.frag");

		_pipeline = HR::GLPipeline::create();
		_pipeline->attachStage(*_vertexShader);
		_pipeline->attachStage(*_fragmentShader);

		std::shared_ptr<IEntity> testEntity = _world->createEntity("TestEntity");

		{
			std::vector<HR::Vertex> vertices{
				HR::Vertex{glm::vec3{-0.75, -0.75, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{1, 0, 0}},
				HR::Vertex{glm::vec3{ 0.75, -0.75, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0}},
				HR::Vertex{glm::vec3{ 0.0,   0.75, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1}},
			};
			std::vector<uint32_t> indices{0, 1, 2};
			testEntity->add(std::unique_ptr<HW::IComponent>(new HC::MeshComponent(std::weak_ptr<IEntity>(testEntity), vertices, indices)));
		}

		BlueprintLoader::save("world.blueprint", "World Blueprint", _world);
	}

	~Engine() final {}

	void run() final {
		while (!_view->isClosed()) {
			_world->tick(HW::TickAction::checkDead);
			_world->tick(HW::TickAction::physics);

			_view->update();

			_renderer->bind(*_view);
			_renderer->use(*_pipeline);
			_renderer->clear(glm::vec4{0, 0.1, 0.1, 1});

			// _renderer->setRenderOrder(HR::RenderOrder::frontToBack);
			_world->tick(HW::TickAction::render);

			// _renderer->setRenderOrder(HR::RenderOrder::backToFront);
			_world->tick(HW::TickAction::renderTransparent);

			_view->finalize();

			_world->tick(HW::TickAction::network);
		}
	}


	HW::IWorld* getWorld() final { return _world.get(); }
	HR::IRenderer* getRenderer() final { return _renderer.get(); }

private:
	std::shared_ptr<HW::IWorld> _world;

	std::unique_ptr<HV::IView> _view;
	std::unique_ptr<HR::IRenderer> _renderer;

	std::unique_ptr<HR::IShader> _vertexShader;
	std::unique_ptr<HR::IShader> _fragmentShader;

	std::unique_ptr<HR::IPipeline> _pipeline;
};

Hydra::IEngine* engineInstance;

int main(int argc, const char** argv) {
	engineInstance = new Engine();
	engineInstance->run();

	delete engineInstance;
	return 0;
}
