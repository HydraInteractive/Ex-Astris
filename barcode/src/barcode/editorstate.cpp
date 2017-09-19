#include <barcode/editorstate.hpp>
#include <barcode/gamestate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <imgui/imgui.h>

namespace Barcode {
	EditorState::EditorState() : _engine(Hydra::IEngine::getInstance()) {}

	void EditorState::load()
	{
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

		int hello = 32434;
	}

	EditorState::~EditorState() { }

	void EditorState::onMainMenu()
	{
		if (ImGui::BeginMenu("Editor"))
		{
			if (ImGui::MenuItem("Import..."))
			{
				_importStatic = !_importStatic;
			}
			ImGui::EndMenu();
		}
	}

	void EditorState::runFrame() {
		{ // Fetch new events
			_engine->getView()->update(_engine->getUIRenderer());
			_engine->getUIRenderer()->newFrame();
		}

		{ // Update physics
			_world->tick(Hydra::World::TickAction::physics);
		}

		{ // Render objects (Deferred rendering)
			_world->tick(Hydra::World::TickAction::render);
		}

		{ // Render transparent objects	(Forward rendering)
			_world->tick(Hydra::World::TickAction::renderTransparent);
		}

		{ // Update UI & views

			_engine->getRenderer()->render(_viewBatch.batch);
			if (_importStatic)
				_importerMenu.render(_importStatic);
		}

		{ // Sync with network
			_world->tick(Hydra::World::TickAction::network);
		}
	}

	void EditorState::_initWorld() {
		_world = Hydra::World::World::create();
		auto a = _world->createEntity("Editor entity");
	}

}
