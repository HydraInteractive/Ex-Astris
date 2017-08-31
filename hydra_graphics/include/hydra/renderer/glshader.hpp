#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/renderer/shader.hpp>

#include <memory>

namespace Hydra::Renderer {
	struct HYDRA_API GLShader final {
		GLShader() = delete;

		static std::unique_ptr<IShader> createFromSource(PipelineStage stage, const std::string& file);

		// TODO: Implement createFromBinary -- https://www.khronos.org/opengl/wiki/Shader_Compilation#Binary_upload
	};

	struct HYDRA_API GLPipeline final {
		GLPipeline() = delete;

		static std::unique_ptr<IPipeline> create();
	};
}
