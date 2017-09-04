/**
 * An implementation for the IShader, and IPipeline, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/renderer/shader.hpp>

#include <memory>

namespace Hydra::Renderer {
	namespace GLShader {
		HYDRA_API std::unique_ptr<IShader> createFromSource(PipelineStage stage, const std::string& file);

		// TODO: Implement createFromBinary -- https://www.khronos.org/opengl/wiki/Shader_Compilation#Binary_upload
	};

	namespace GLPipeline {
		HYDRA_API std::unique_ptr<IPipeline> create();
	};
}
