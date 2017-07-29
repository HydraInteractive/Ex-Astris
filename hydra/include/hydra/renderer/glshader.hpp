#pragma once

#include <hydra/renderer/shader.hpp>

#include <memory>

namespace Hydra::Renderer {
	struct GLShader final {
		GLShader() = delete;

		static std::unique_ptr<IShader> createFromSource(PipelineStage stage, const std::string& file);

		// TODO: Implement createFromBinary -- https://www.khronos.org/opengl/wiki/Shader_Compilation#Binary_upload
	};

	struct GLPipeline final {
		GLPipeline() = delete;

		static std::unique_ptr<IPipeline> create();
	};
}
