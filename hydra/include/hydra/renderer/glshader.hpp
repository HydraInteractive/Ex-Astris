#pragma once

#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	struct GLShader final {
		GLShader() = delete;

		IShader* createFromSource(PipelineStage stage, const std::string& file);

		// TODO: Implement createFromBinary
	};

	struct GLPipeline final {
		GLPipeline() = delete;

		IPipeline* create();
	};
}
