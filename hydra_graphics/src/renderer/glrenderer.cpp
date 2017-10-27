// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IRenderer, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

// NOTE: This GLRenderer depends on SDL aswell!
//       Maybe abstract way this in the future.
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <hydra/engine.hpp>
#include <hydra/ext/stacktrace.hpp>

using namespace Hydra::Renderer;

static void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

class GLRendererImpl final : public IRenderer {
public:
	GLRendererImpl(Hydra::View::IView& view) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		_glContext = SDL_GL_CreateContext(_window = static_cast<SDL_Window*>(view.getHandler()));
		_loadGLAD();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&glDebugLog, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		SDL_GL_SetSwapInterval(0);

		_fullscreenQuad = Hydra::Renderer::GLMesh::createFullscreenQuad();

		glGenBuffers(1, &_modelMatrixBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
		glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, NULL, GL_STREAM_DRAW);

		glGenBuffers(1, &_particleBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _particleBuffer);
		glBufferData(GL_ARRAY_BUFFER, _particleBufferSize, NULL, GL_STREAM_DRAW);
	}

	~GLRendererImpl() final {
		glDeleteBuffers(1, &_modelMatrixBuffer);
		glDeleteBuffers(1, &_particleBuffer);
		SDL_GL_DeleteContext(_glContext);
	}


	void renderAnimation(Batch& batch) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));


		batch.pipeline->setValue(7, 0);
		batch.pipeline->setValue(8, 1);
		batch.pipeline->setValue(9, 2);
		batch.pipeline->setValue(10, 3);
		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;
			mesh->getMaterial().diffuse->bind(0);
			mesh->getMaterial().normal->bind(1);
			mesh->getMaterial().specular->bind(2);
			mesh->getMaterial().glow->bind(3);
			glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
			glBindVertexArray(mesh->getID());
			size_t size = kv.second.size();
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			for (size_t i = 0; i < size; i += maxPerLoop) {
				int currentFrame = mesh->getCurrentKeyframe();
				if (mesh->getAnimationCounter() > 24 && currentFrame < mesh->getMaxFramesForAnimation()) {
					mesh->getAnimationCounter() = 0;
					mesh->setCurrentKeyframe(currentFrame + 1);
				}

				glm::mat4 tempMat;
				for (int i = 0; i < mesh->getNrOfJoints(); i++) {
					tempMat = mesh->getTransformationMatrices(i);
					batch.pipeline->setValue(11 + i, tempMat);
				}

				size_t amount = std::min(size - i, maxPerLoop);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
			}
		}
	}

	void renderShadows(Batch& batch) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));

		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;

			batch.pipeline->setValue(2, mesh->hasAnimation());

			if (mesh->hasAnimation()) {
				glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
				glBindVertexArray(mesh->getID());
				size_t size = kv.second.size();
				const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
				for (size_t i = 0; i < size; i += maxPerLoop) {

					int currentFrame = mesh->getCurrentKeyframe();
					if (currentFrame < mesh->getMaxFramesForAnimation()) {
						mesh->setCurrentKeyframe(currentFrame + 1);
					}
					else {
						mesh->setCurrentKeyframe(1);
					}

					glm::mat4 tempMat;
					for (int i = 0; i < mesh->getNrOfJoints(); i++) {
						tempMat = mesh->getTransformationMatrices(i);
						batch.pipeline->setValue(11 + i, tempMat);
					}

					size_t amount = std::min(size - i, maxPerLoop);
					glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
					glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
				}
			}
			else {
				size_t size = kv.second.size();
				const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
				for (size_t i = 0; i < size; i += maxPerLoop) {
					size_t amount = std::min(size - i, maxPerLoop);
					glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
					glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
					glBindVertexArray(mesh->getID());
					glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
				}
			}
		}
	}


	void render(ParticleBatch& batch) final { // For particles only.
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);
		auto& particles = batch.textureInfo;
		size_t sizeParticles = particles.size() / 3;
		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			glBindVertexArray(mesh->getID());
			for (size_t i = 0; i < sizeParticles; i+= maxPerLoop) {
				size_t amount = std::min(sizeParticles - i, maxPerLoop);
				glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);

				glBindBuffer(GL_ARRAY_BUFFER, _particleBuffer);
				glBufferData(GL_ARRAY_BUFFER, _particleBufferSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::vec2) * 3, &particles[i*3]);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
			}
		}
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	void render(Batch& batch) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));

		batch.pipeline->setValue(20, 0);
		batch.pipeline->setValue(21, 1);
		batch.pipeline->setValue(22, 2);
		batch.pipeline->setValue(23, 3);
		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;
			mesh->getMaterial().diffuse->bind(0);
			mesh->getMaterial().normal->bind(1);
			mesh->getMaterial().specular->bind(2);
			mesh->getMaterial().glow->bind(3);
			size_t size = kv.second.size();
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			for (size_t i = 0; i < size; i += maxPerLoop) {
				size_t amount = std::min(size - i, maxPerLoop);
				glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glBindVertexArray(mesh->getID());
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
			}
		}
	}

	void postProcessing(Batch& batch) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));

		glDisable(GL_CULL_FACE);
		glBindVertexArray(_fullscreenQuad->getID());
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_fullscreenQuad->getIndicesCount()), GL_UNSIGNED_INT, nullptr, 1);
		glEnable(GL_CULL_FACE);
	}

	DrawObject* aquireDrawObject() final {
		std::unique_ptr<DrawObject> drawObj;
		DrawObject* drawObjPtr;

		if (_inactiveDrawObjects.empty())
			drawObj = std::make_unique<DrawObject>();
		else {
			drawObj = std::move(_inactiveDrawObjects.back());
			_inactiveDrawObjects.pop_back();
		}

		drawObjPtr = drawObj.get();
		_activeDrawObjects.push_back(std::move(drawObj));
		return drawObjPtr;
	}

	const std::vector<std::unique_ptr<DrawObject>>& activeDrawObjects() final { return _activeDrawObjects; }

	void clear(Batch& batch) final{
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);
	}

	void cleanup() final {
		auto isInactive = [this](auto& drawObj) {
			if (drawObj->refCounter)
				return false;
			_inactiveDrawObjects.push_back(std::move(drawObj));
			return true;
		};

		_activeDrawObjects.erase(std::remove_if(_activeDrawObjects.begin(), _activeDrawObjects.end(), isInactive), _activeDrawObjects.end());
	}

	void* getModelMatrixBuffer() final { return static_cast<void*>(&_modelMatrixBuffer); }
	void* getParticleExtraBuffer() final { return static_cast<void*>(&_particleBuffer); }

private:
	SDL_Window* _window;
	SDL_GLContext _glContext;
	std::vector<std::unique_ptr<DrawObject>> _activeDrawObjects;
	std::vector<std::unique_ptr<DrawObject>> _inactiveDrawObjects;
	std::unique_ptr<IMesh> _fullscreenQuad;

	const size_t _modelMatrixSize = sizeof(glm::mat4) * 128; // max 128 mesh instances per draw call
	GLuint _modelMatrixBuffer;
	GLuint _particleBuffer;
	const size_t _particleBufferSize = sizeof(glm::vec2) * 3 * 128; // Particle buffer holds three vec2, and max 128 particle instances per draw call.
 
	static void _loadGLAD() {
		static bool initialized = false;
		if (!initialized) {
			gladLoadGLLoader(SDL_GL_GetProcAddress);
			initialized = true;
		}
	}
};

std::unique_ptr<IRenderer> GLRenderer::create(Hydra::View::IView& view) {
	return std::unique_ptr<IRenderer>(new ::GLRendererImpl(view));
}

void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
	if(id == 4 || id == 8 || id == 20 || id == 36 || id == 37 || id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	std::string sourceStr = "!UNKNOWN!";
	if (source == GL_DEBUG_SOURCE_API)
		sourceStr = "API";
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		sourceStr = "WINDOW_SYSTEM";
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
		sourceStr = "SHADER_COMPILER";
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
		sourceStr = "THIRD_PARTY";
	else if (source == GL_DEBUG_SOURCE_APPLICATION)
		sourceStr = "APPLICATION";
	else if (source == GL_DEBUG_SOURCE_OTHER)
		sourceStr = "OTHER";

	std::string typeStr = "!UNKNOWN!";

	if (type == GL_DEBUG_TYPE_ERROR)
		typeStr = "ERROR";
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		typeStr = "DEPRECATED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		typeStr = "UNDEFINED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_PORTABILITY)
		typeStr = "PORTABILITY";
	else if (type == GL_DEBUG_TYPE_PERFORMANCE)
		typeStr = "PERFORMANCE";
	else if (type == GL_DEBUG_TYPE_MARKER)
		typeStr = "MARKER";
	else if (type == GL_DEBUG_TYPE_PUSH_GROUP)
		typeStr = "PUSH_GROUP";
	else if (type == GL_DEBUG_TYPE_POP_GROUP)
		typeStr = "POP_GROUP";
	else if (type == GL_DEBUG_TYPE_OTHER)
		typeStr = "OTHER";

	std::string severityStr = "!UNKNOWN!";

	Hydra::LogLevel level;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		severityStr = "HIGH";
		level = Hydra::LogLevel::error;
	} else if (severity == GL_DEBUG_SEVERITY_MEDIUM){
		severityStr = "MEDIUM";
		level = Hydra::LogLevel::warning;
	} else if (severity == GL_DEBUG_SEVERITY_LOW) {
		severityStr = "LOW";
		level = Hydra::LogLevel::normal;
	} else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		severityStr = "NOTIFICATION";
		level = Hydra::LogLevel::verbose;
	}

	std::string stackTrace = Hydra::Ext::getStackTrace();

	Hydra::IEngine::getInstance()->log(level, "GL error: Source %s, Type: %s, ID: %d, Severity: %s\n%s%s%s", sourceStr.c_str(), typeStr.c_str(), id, severityStr.c_str(), message, stackTrace.length() ? "\n" : "", stackTrace.c_str());
	//if (level == Hydra::LogLevel::error)
	//	exit(0);
}