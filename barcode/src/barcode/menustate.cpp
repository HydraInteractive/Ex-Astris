#include <barcode/menustate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>
#include <hydra/physics/bulletmanager.hpp>

#include <barcode/gamestate.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	MenuState::MenuState() : _engine(Hydra::IEngine::getInstance()) {}

	void MenuState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());
		_physicsManager = Hydra::Physics::BulletManager::create();

		{
			auto& batch = _viewBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/view.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/view.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0.5, 0, 0.5, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = _engine->getView();
			batch.batch.pipeline = batch.pipeline.get();
		}

		_initSystem();
		_initWorld();
	}

	MenuState::~MenuState() { }

	void MenuState::onMainMenu() {}

	void MenuState::runFrame(float delta) {
		deadSystem.tick(0);

		{ // Fetch new events
			_engine->getView()->update(_engine->getUIRenderer());
			_engine->getUIRenderer()->newFrame();
		}

		{ // Update UI & views
			ImGui::Begin("Main menu");
			/*if (ImGui::Button("Play game"))
				_engine->setState<GameState>();*/

			if (ImGui::Button("Quit"))
				_engine->quit();
			ImGui::End();

			_engine->getRenderer()->render(_viewBatch.batch);
		}
	}

	void MenuState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = {&deadSystem};
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void MenuState::_initWorld() {
		using world = Hydra::World::World;

		world::newEntity("Menu entity", world::root);
	}

}
