#include <hydra/renderer/glshader.hpp>

#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

static GLuint stageToGL(Hydra::Renderer::PipelineStage stage) {
	using Hydra::Renderer::PipelineStage;
	switch (stage) {
	case PipelineStage::vertex:
		return GL_VERTEX_SHADER;
	case PipelineStage::geometry:
		return GL_GEOMETRY_SHADER;
	case PipelineStage::fragment:
		return GL_FRAGMENT_SHADER;
	default:
		throw "UNKNOWN PIPELINESTAGE";
	}
}

static GLuint stageToGLBit(Hydra::Renderer::PipelineStage stage) {
	using Hydra::Renderer::PipelineStage;

	GLuint result = 0;
	if ((stage & PipelineStage::vertex) == PipelineStage::vertex)
		result |= GL_VERTEX_SHADER_BIT;
	if ((stage & PipelineStage::geometry) == PipelineStage::geometry)
		result |= GL_GEOMETRY_SHADER_BIT;
	if ((stage & PipelineStage::fragment) == PipelineStage::fragment)
		result |= GL_FRAGMENT_SHADER_BIT;

	return result;
}

class GLShader : public Hydra::Renderer::IShader {
public:
	GLShader(Hydra::Renderer::PipelineStage stage, const std::string& source) {
		const char * src = source.c_str();
		_program = glCreateShaderProgramv(stageToGL(stage), 1, &src);
	}

	~GLShader() final {
		glDeleteProgram(_program);
	}

	void setValue(int32_t id, int value) final { glProgramUniform1i(_program, id, value); }
	void setValue(int32_t id, float value) final { glProgramUniform1f(_program, id, value); }
	void setValue(int32_t id, const glm::ivec2& value) final { glProgramUniform2iv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::ivec3& value) final { glProgramUniform3iv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::ivec4& value) final { glProgramUniform4iv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::vec2& value) final { glProgramUniform2fv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::vec3& value) final { glProgramUniform3fv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::vec4& value) final { glProgramUniform4fv(_program, id, 1, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::mat2& value) final { glProgramUniformMatrix2fv(_program, id, 1, false, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::mat3& value) final { glProgramUniformMatrix3fv(_program, id, 1, false, glm::value_ptr(value)); }
	void setValue(int32_t id, const glm::mat4& value) final { glProgramUniformMatrix4fv(_program, id, 1, false, glm::value_ptr(value)); }

	void* getHandler() final { return static_cast<void*>(&_program); }

private:
	GLuint _program;
};

class GLPipeline : public Hydra::Renderer::IPipeline {
public:
	GLPipeline() {
		glGenProgramPipelines(1, &_pipeline);
	}

	~GLPipeline() final {
		glDeleteProgramPipelines(1, &_pipeline);
	}

	void attachStage(Hydra::Renderer::PipelineStage stage, Hydra::Renderer::IShader* shader) final { glUseProgramStages(_pipeline, stageToGLBit(stage), *static_cast<GLuint*>(shader->getHandler())); }
	void detachStage(Hydra::Renderer::PipelineStage stage) final { glUseProgramStages(_pipeline, stageToGLBit(stage), 0); }

	void bind() final { glBindProgramPipeline(_pipeline); }

private:
	GLuint _pipeline;
};


Hydra::Renderer::IShader* Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage stage, const std::string& file) {
	FILE* fp = fopen(file.c_str(), "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* data = static_cast<char*>(malloc(length));
	fread(data, length, 1, fp);
	fclose(fp);

	std::string src = std::string(data, length);
	free(data);

	return static_cast<Hydra::Renderer::IShader*>(new ::GLShader(stage, src));
}

Hydra::Renderer::IPipeline* Hydra::Renderer::GLPipeline::create() {
	return static_cast<Hydra::Renderer::IPipeline*>(new ::GLPipeline());
}
