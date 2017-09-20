// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IShader, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/glshader.hpp>

#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <hydra/engine.hpp>

using namespace Hydra;
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


class GLShaderImpl : public IShader {
public:
	GLShaderImpl(PipelineStage stage, const std::string& file) : _stage(stage) {
		IEngine::getInstance()->log(LogLevel::verbose, "Loading shader: %s", file.c_str());
		FILE* fp = fopen(file.c_str(), "rb");
		if (!fp) {
			IEngine::getInstance()->log(LogLevel::error, "Shader does not exist: %s", file.c_str());
			return;
		}

		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* data = static_cast<char*>(malloc(length + 1));
		if (!data) {
			IEngine::getInstance()->log(LogLevel::error, "Failed malloc");
			return;
		}
		fread(data, length, 1, fp);
		fclose(fp);
		data[length] = '\0';

		_shader = glCreateShader(stageToGL(stage));
		glShaderSource(_shader, 1, &data, nullptr);
		glCompileShader(_shader);

		GLint status;
		glGetShaderiv(_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint len;
			glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* errorLog = static_cast<GLchar*>(malloc(len));
			glGetShaderInfoLog(_shader, len, &len, errorLog);

			IEngine::getInstance()->log(LogLevel::error, "Shader compilation failed %s (%u):\n%s", file.c_str(), _shader, errorLog);
			free(errorLog);
		}

		free(data);
	}

	~GLShaderImpl() final {
		glDeleteShader(_shader);
	}

	PipelineStage getStage() final { return _stage; }

	void* getHandler() final { return static_cast<void*>(&_shader); }

private:
	PipelineStage _stage;
	GLuint _shader;
};

class GLPipelineImpl : public IPipeline {
public:
	GLPipelineImpl() {
		_program = glCreateProgram();
	}

	~GLPipelineImpl() final {
		glDeleteProgram(_program);
	}

	void attachStage(IShader& shader) final { glAttachShader(_program, *static_cast<GLuint*>(shader.getHandler())); }

	void finalize() final {
		glLinkProgram(_program);

		GLint status;
		glGetProgramiv(_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint len;
			glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &len);

			GLchar* errorLog = static_cast<GLchar*>(malloc(len));
			glGetProgramInfoLog(_program, len, &len, errorLog);

			IEngine::getInstance()->log(LogLevel::error, "Linking the program failed %u:\n%s", _program, errorLog);
			free(errorLog);
		}
	}

	void setValue(int32_t id, int32_t value) final { glProgramUniform1i(_program, id, value); }
	void setValue(int32_t id, uint32_t value) final { glProgramUniform1ui(_program, id, value); }
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
	void setValue(int32_t id, const glm::mat4& value, int i, std::string path) final { glProgramUniformMatrix4fv(glGetUniformLocation(_program, (path + "[" + std::to_string(i) + "]").c_str()), id, 1, false, glm::value_ptr(value)); }
	

	void* getHandler() final { return static_cast<void*>(&_program); }

private:
	GLuint _program;
};

std::unique_ptr<IShader> GLShader::createFromSource(PipelineStage stage, const std::string& file) {
	return std::unique_ptr<IShader>(new ::GLShaderImpl(stage, file));
}

std::unique_ptr<IPipeline> GLPipeline::create() {
	return std::unique_ptr<IPipeline>(new ::GLPipelineImpl());
}
