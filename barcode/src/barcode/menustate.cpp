#include <barcode/menustate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <barcode/gamestate.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	MenuState::MenuState() : _engine(Hydra::IEngine::getInstance()) {}

	void MenuState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

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

		_initWorld();
	}

	MenuState::~MenuState() { }

	void MenuState::runFrame(float delta) {
		{ // Fetch new events
			_engine->getView()->update(_engine->getUIRenderer());
			_engine->getUIRenderer()->newFrame();
		}

		{ // Update physics
			_world->tick(Hydra::World::TickAction::physics, delta);
		}

		{ // Render objects (Deferred rendering)
			_world->tick(Hydra::World::TickAction::render, delta);
		}

		{ // Render transparent objects	(Forward rendering)
			_world->tick(Hydra::World::TickAction::renderTransparent, delta);
		}

		{ // Update UI & views
			ImGui::Begin("Main menu");
			if (ImGui::Button("Play game"))
				_engine->setState<GameState>();

			if (ImGui::Button("Quit"))
				_engine->quit();
			ImGui::End();

			_engine->getRenderer()->render(_viewBatch.batch);
		}

		{ // Sync with network
			_world->tick(Hydra::World::TickAction::network, delta);
		}
	}

	void MenuState::_initWorld() {
		_world = Hydra::World::World::create();
		auto a = _world->createEntity("Menu entity");
	}

}
