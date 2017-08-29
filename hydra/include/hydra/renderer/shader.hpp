#pragma once
#include <hydra/ext/api.hpp>

#include <string>
#include <cstdint>

#include <glm/glm.hpp>

namespace Hydra::Renderer {
	enum class HYDRA_API PipelineStage : uint32_t {
		vertex = 1 << 0,
		geometry = 1 << 1,
		fragment = 1 << 2
	};

	inline PipelineStage operator& (PipelineStage a, PipelineStage b) { return static_cast<PipelineStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	class HYDRA_API IShader {
	public:
		virtual ~IShader() = 0;

		virtual void setValue(int32_t id, int value) = 0;
		virtual void setValue(int32_t id, float value) = 0;
		virtual void setValue(int32_t id, const glm::ivec2& value) = 0;
		virtual void setValue(int32_t id, const glm::ivec3& value) = 0;
		virtual void setValue(int32_t id, const glm::ivec4& value) = 0;
		virtual void setValue(int32_t id, const glm::vec2& value) = 0;
		virtual void setValue(int32_t id, const glm::vec3& value) = 0;
		virtual void setValue(int32_t id, const glm::vec4& value) = 0;
		virtual void setValue(int32_t id, const glm::mat2& value) = 0;
		virtual void setValue(int32_t id, const glm::mat3& value) = 0;
		virtual void setValue(int32_t id, const glm::mat4& value) = 0;

		virtual PipelineStage getStage() = 0;

		virtual void* getHandler() = 0;
	};
	inline IShader::~IShader() {}

	class HYDRA_API IPipeline {
	public:
		virtual ~IPipeline() = 0;

		virtual void attachStage(IShader& shader) = 0;
		virtual void detachStage(PipelineStage stage) = 0;

		virtual void* getHandler() = 0;
	};
	inline IPipeline::~IPipeline() {}
}
