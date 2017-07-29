#include <hydra/engine.hpp>

#include <memory>

#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>

namespace HV = Hydra::View;
namespace HR = Hydra::Renderer;

class Engine final : public Hydra::Engine {
public:
	Engine() {
		_view = HV::SDLView::create();
		_renderer = HR::GLRenderer::create(*_view);

		_vertexShader = HR::GLShader::createFromSource(HR::PipelineStage::vertex, "assets/shaders/base.vert");
		_fragmentShader = HR::GLShader::createFromSource(HR::PipelineStage::fragment, "assets/shaders/base.frag");

		_pipeline = HR::GLPipeline::create();
		_pipeline->attachStage(*_vertexShader);
		_pipeline->attachStage(*_fragmentShader);

		{
			std::vector<HR::Vertex> vertices{
				HR::Vertex{glm::vec3{-0.5, -0.5, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{1, 0, 0}},
				HR::Vertex{glm::vec3{ 0.5, -0.5, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0}},
				HR::Vertex{glm::vec3{ 0.0,  0.5, 0.0}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1}},
			};
			std::vector<uint32_t> indices{0, 1, 2};
			_mesh = HR::GLMesh::create(vertices, indices);
		}
	}

	virtual ~Engine() final {
	}

	void run() final {
		while (!_view->isClosed()) {
			_view->update();

			_renderer->bind(*_view);
			_renderer->use(*_pipeline);
			_renderer->clear(glm::vec4{0, 0.2, 0.2, 1});

			_renderer->render(*_mesh, 1);

			_view->finalize();
		}
	}

private:
	std::unique_ptr<HV::IView> _view;
	std::unique_ptr<HR::IRenderer> _renderer;

	std::unique_ptr<HR::IShader> _vertexShader;
	std::unique_ptr<HR::IShader> _fragmentShader;

	std::unique_ptr<HR::IPipeline> _pipeline;

	std::unique_ptr<HR::IMesh> _mesh;
};

int main(int argc, const char** argv) {
	Engine().run();
	return 0;
}
