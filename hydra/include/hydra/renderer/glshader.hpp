#pragma once

#include <hydra/renderer/shader.hpp>

namespace Hydra::Renderer {
	struct GLShader final {
		IShader* createFromSource(PipelineStage stage, const std::string& file);

		// TODO: Implement createFromBinary
	};

	struct GLPipeline final {
		IPipeline* create();
	};
}
