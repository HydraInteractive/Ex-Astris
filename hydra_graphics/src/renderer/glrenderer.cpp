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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <hydra/engine.hpp>
#include <hydra/ext/stacktrace.hpp>
#include <imgui/imgui.h>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/transformcomponent.hpp>

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
		// TODO: Does it need sizeof(float)?
		_animationTransTexture = Hydra::Renderer::GLTexture::createDataTexture(100 * 16 * sizeof(float), _maxInstancedAnimatedModels, Hydra::Renderer::TextureType::f16RGBA);

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


	void renderAnimation(AnimationBatch& batch) final {
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
		batch.pipeline->setValue(11, 4);
		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;
			mesh->getMaterial().diffuse->bind(0);
			mesh->getMaterial().normal->bind(1);
			mesh->getMaterial().specular->bind(2);
			mesh->getMaterial().glow->bind(3);

			glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
			glBindVertexArray(mesh->getID());

			constexpr unsigned int h = 1; // Width is always going to be 1
			auto& currentFrames = batch.currentFrames[mesh];
			auto& currAnimIndices = batch.currAnimIndices[mesh];
			const size_t maxPerLoop = _maxInstancedAnimatedModels;
			size_t size = batch.currentFrames[mesh].size();
			for (size_t i = 0; i < size; i += maxPerLoop) {
				for (size_t instanceIdx = i; instanceIdx < i + maxPerLoop && instanceIdx < size; instanceIdx++) {
					unsigned int w = mesh->getNrOfJoints(currAnimIndices[instanceIdx]) * 16 * 4;
					std::vector<glm::mat4> jointTransformMX;
					int frame = currentFrames[instanceIdx];
					int animIdx = currAnimIndices[instanceIdx];
					for (int currJoint = 0; currJoint < mesh->getNrOfJoints(currAnimIndices[instanceIdx]); currJoint++) {
						jointTransformMX.push_back(mesh->getTransformationMatrices(animIdx, currJoint, frame));
					}
					_animationTransTexture->setData(glm::ivec2(0, instanceIdx), glm::ivec2(w, h), jointTransformMX.data());
				}

				_animationTransTexture->bind(4);

				size_t amount = std::min(size - i, maxPerLoop);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
			}
		}
	}

	void renderShadows(AnimationBatch& batch) final {
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

			glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
			glBindVertexArray(mesh->getID());

			batch.pipeline->setValue(20, 0);

			constexpr unsigned int h = 1; // Width is always going to be 1
			auto& currentFrames = batch.currentFrames[mesh];
			auto& currAnimIndices = batch.currAnimIndices[mesh];
			const size_t maxPerLoop = _maxInstancedAnimatedModels;
			size_t size = batch.currentFrames[mesh].size();

			for (size_t i = 0; i < size; i += maxPerLoop) {
				for (size_t instanceIdx = i; instanceIdx < i + maxPerLoop && instanceIdx < size; instanceIdx++) {
					unsigned int w = mesh->getNrOfJoints(currAnimIndices[instanceIdx]) * 16 * 4;
					std::vector<glm::mat4> jointTransformMX;
					int frame = currentFrames[instanceIdx];
					int animIdx = currAnimIndices[instanceIdx];
					for (int currJoint = 0; currJoint < mesh->getNrOfJoints(currAnimIndices[instanceIdx]); currJoint++) {
						jointTransformMX.push_back(mesh->getTransformationMatrices(animIdx, currJoint, frame));
					}
					_animationTransTexture->setData(glm::ivec2(0, instanceIdx), glm::ivec2(w, h), jointTransformMX.data());
				}
				_animationTransTexture->bind(0);

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

			glBindVertexArray(mesh->getID());
			glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
			size_t size = kv.second.size();
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			for (size_t i = 0; i < size; i += maxPerLoop) {
				size_t amount = std::min(size - i, maxPerLoop);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
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
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	void renderHitboxes(Batch& batch) {
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
		for (auto& kv : batch.objects) {
			auto& mesh = kv.first;
			size_t size = kv.second.size();
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			for (size_t i = 0; i < size; i += maxPerLoop) {
				size_t amount = std::min(size - i, maxPerLoop);
				glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
				glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, nullptr, GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glBindVertexArray(mesh->getID());
				glDrawElementsInstanced(GL_LINES, static_cast<GLsizei>(mesh->getIndicesCount()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount));
			}
		}
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
		*drawObjPtr = DrawObject();
		_activeDrawObjects.push_back(std::move(drawObj));
		return drawObjPtr;
	}

	void showGuizmo() final {
		auto& this_ = Hydra::Component::TransformComponent::_currentlyEditing;
		if (!this_)
			return;

		auto p = this_->_getParentComponent();
		glm::mat4 parent = p ? glm::inverseTranspose(p->getMatrix()) : glm::mat4(1);
		//this_->_recalculateMatrix();
		glm::mat4 matrix = this_->getMatrix();//_matrix;//glm::translate(this_->position) * glm::mat4_cast(glm::normalize(this_->rotation)) * glm::scale(this_->scale);

		static ImGuizmo::OPERATION currentOperation(ImGuizmo::ROTATE);
		static ImGuizmo::MODE currentMode(ImGuizmo::WORLD);

		ImGui::Begin("Guizmo tools");
		ImGui::Text("Currently editing: %s", Hydra::World::World::getEntity(this_->entityID)->name.c_str());

		if (ImGui::IsKeyPressed(SDLK_z))
			currentOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(SDLK_x))
			currentOperation = ImGuizmo::ROTATE;
		/*if (ImGui::IsKeyPressed(SDLK_c))
			currentOperation = ImGuizmo::SCALE;*/

		ImGui::Text("<Z> Translate, <X> Rotate"); // , <C> Scale

		if (ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
			currentOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
			currentOperation = ImGuizmo::ROTATE;
		/*ImGui::SameLine();
			if (ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
			currentOperation = ImGuizmo::SCALE;*/

		if (currentOperation != ImGuizmo::SCALE) {
			if (ImGui::RadioButton("Local", currentMode == ImGuizmo::LOCAL))
				currentMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", currentMode == ImGuizmo::WORLD))
				currentMode = ImGuizmo::WORLD;
		}

		static bool useSnap(false);
		if (ImGui::IsKeyPressed(83))
			useSnap = !useSnap;
		ImGui::Checkbox("", &useSnap);
		ImGui::SameLine();

		glm::vec3* snap = nullptr;
		switch (currentOperation) {
		case ImGuizmo::TRANSLATE: {
			static glm::vec3 snapTranslation = glm::vec3(0.1);
			snap = &snapTranslation;
			ImGui::InputFloat3("Snap", glm::value_ptr(snapTranslation));
			break;
		}
		case ImGuizmo::ROTATE: {
			static glm::vec3 snapRotation = glm::vec3(0.1);
			snap = &snapRotation;
			ImGui::InputFloat("Angle Snap", glm::value_ptr(snapRotation));
			break;
		}
		case ImGuizmo::SCALE: {
			/*static glm::vec3 snapScale = glm::vec3(0.1);
				snap = &snapScale;
				ImGui::InputFloat("Scale Snap", glm::value_ptr(snapScale));*/
			break;
		}
		}
		ImGui::End();

		//TODO: Make argument
		auto* cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());

		glm::mat4 deltaMatrix;
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::Manipulate(glm::value_ptr(cc->getViewMatrix()), glm::value_ptr(cc->getProjectionMatrix()), currentOperation, currentMode, glm::value_ptr(matrix), glm::value_ptr(deltaMatrix), useSnap ? glm::value_ptr(*snap) : nullptr);
		this_->dirty = true;
		matrix *= parent;

		static bool didUse = false;
		if (ImGuizmo::IsUsing())/*
															didUse = true;
															else if (didUse)*/ {
			didUse = false;
			switch(currentOperation) {
			case ImGuizmo::TRANSLATE:
				this_->position += glm::vec3(deltaMatrix[3]);
				break;
			case ImGuizmo::ROTATE:
				this_->rotation = glm::toQuat(deltaMatrix) * this_->rotation;
				break;
			case ImGuizmo::SCALE:
				/*//this_->scale += glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]); //glm::scale(matrix);
					glm::vec3 translation;
					glm::vec3 rotation;
					glm::vec3 scale;
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
					this_->scale = scale;
					// this_->scale *= glm::vec3(matrix[0][0], matrix[1][1], matrix[2][2]);*/
				break;
			}
		}
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
	std::shared_ptr<Hydra::Renderer::ITexture> _animationTransTexture;

	const size_t _modelMatrixSize = sizeof(glm::mat4) * 128; // max 128 mesh instances per draw call
	GLuint _modelMatrixBuffer;
	GLuint _particleBuffer;
	const size_t _particleBufferSize = sizeof(glm::vec2) * 3 * 128; // Particle buffer holds three vec2, and max 128 particle instances per draw call.
	const int _maxInstancedAnimatedModels = 20;

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
	if(id == 4 || id == 8 || id == 20 || id == 36 || id == 37 || id == 1282 || id == 131169 || id == 131185 || id == 131218 || id == 131204)
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
