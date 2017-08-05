#include <hydra/renderer/glshader.hpp>

#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Renderer;

static GLuint stageToGL(PipelineStage stage) {
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

static GLuint stageToGLBit(PipelineStage stage) {
	GLuint result = 0;
	if ((stage & PipelineStage::vertex) == PipelineStage::vertex)
		result |= GL_VERTEX_SHADER_BIT;
	if ((stage & PipelineStage::geometry) == PipelineStage::geometry)
		result |= GL_GEOMETRY_SHADER_BIT;
	if ((stage & PipelineStage::fragment) == PipelineStage::fragment)
		result |= GL_FRAGMENT_SHADER_BIT;

	return result;
}

class GLShaderImpl : public IShader {
public:
	GLShaderImpl(PipelineStage stage, const std::string& source) : _stage(stage) {
		const char * src = source.c_str();
		_program = glCreateShaderProgramv(stageToGL(stage), 1, &src);

		GLint status;
		glGetProgramiv(_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint len;
			glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &len);

			GLchar* errorLog = static_cast<GLchar*>(malloc(len));
			glGetProgramInfoLog(_program, len, &len, errorLog);

			char buf[0x1000];
			snprintf(buf, sizeof(buf), "Linking failed (%u):\n%s", _program, errorLog);
			free(errorLog);

			fprintf(stderr, "%s", buf);
			throw buf;
		}
	}

	~GLShaderImpl() final {
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

	PipelineStage getStage() final { return _stage; }

	void* getHandler() final { return static_cast<void*>(&_program); }

private:
	PipelineStage _stage;
	GLuint _program;
};

class GLPipelineImpl : public IPipeline {
public:
	GLPipelineImpl() {
		glGenProgramPipelines(1, &_pipeline);
	}

	~GLPipelineImpl() final {
		glDeleteProgramPipelines(1, &_pipeline);
	}

	void attachStage(IShader& shader) final { glUseProgramStages(_pipeline, stageToGLBit(shader.getStage()), *static_cast<GLuint*>(shader.getHandler())); }

	void detachStage(PipelineStage stage) final { glUseProgramStages(_pipeline, stageToGLBit(stage), 0); }

	void* getHandler() final { return static_cast<void*>(&_pipeline); }

private:
	GLuint _pipeline;
};


std::unique_ptr<IShader> GLShader::createFromSource(PipelineStage stage, const std::string& file) {
	FILE* fp = fopen(file.c_str(), "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* data = static_cast<char*>(malloc(length + 1));
	fread(data, length, 1, fp);
	fclose(fp);
	data[length] = '\0';

	std::string src = std::string(data, length);
	free(data);

	return std::unique_ptr<IShader>(new ::GLShaderImpl(stage, src));
}

std::unique_ptr<IPipeline> GLPipeline::create() {
	return std::unique_ptr<IPipeline>(new ::GLPipelineImpl());
}
