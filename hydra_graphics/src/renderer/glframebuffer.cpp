// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IFramebuffer, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/glrenderer.hpp>

#include <hydra/engine.hpp>

#include <glad/glad.h>

#include <map>

using namespace Hydra::Renderer;

class GLFramebufferImpl final : public IFramebuffer {
public:
	GLFramebufferImpl(glm::ivec2 size, size_t samples) : _size(size), _samples(samples) {
		GLuint fbos[2];
		glGenFramebuffers(sizeof(fbos) / sizeof(*fbos), fbos);
		_framebuffer = fbos[0];
		_resolverFramebuffer = fbos[1];
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	}

	~GLFramebufferImpl() final {
		GLuint fbos[2] = {_framebuffer, _resolverFramebuffer};
		glDeleteFramebuffers(sizeof(fbos) / sizeof(*fbos), fbos);
	}

	// ITexture
	void resize(glm::ivec2 size) final {
		_size = size;
		for (auto& attachment : _attachments)
			attachment.second->resize(size);

		if (_depth)
			_depth->resize(size);
	}

	size_t getSamples() final { return _samples; }
	glm::ivec2 getSize() final { return _size; }
	uint32_t getID() const final { return _framebuffer; }

	// IFramebuffer
	IFramebuffer& addTexture(size_t id, TextureType type) final {
		auto texture = GLTexture::createEmpty(_size.x, _size.y, type, _samples);

		if (static_cast<uint32_t>(type) >= static_cast<uint32_t>(TextureType::f16Depth)) {
			// The imager will be bound because of createEmpty
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//GLfloat border[4] = { 0,0,0,0 };
			//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->getID(), 0);
			glDrawBuffer(GL_NONE);
			_depth = texture;
		} else {
			glFramebufferTexture(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + id), texture->getID(), 0);
			_attachments[static_cast<GLuint>(id)] = texture;
		}

		return *this;
	}

	void finalize() final {
		using namespace Hydra;
		std::vector<GLenum> buffers;
		for (auto it = _attachments.begin(); it != _attachments.end(); ++it)
			buffers.push_back(GL_COLOR_ATTACHMENT0 + it->first);

		glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			IEngine::getInstance()->log(LogLevel::error, "Framebuffer failed! Status: %x", status);
	}

	std::shared_ptr<ITexture> getDepth() final { return _depth; }

	//TODO: Add resolve to FBO and not just a texture
	std::shared_ptr<ITexture> resolve(size_t idx, std::shared_ptr<ITexture> result) final {
		result->resize(_size);

		glBindFramebuffer(GL_FRAMEBUFFER, _resolverFramebuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result->getID(), 0);
		GLenum buffers = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &buffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolverFramebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
		glReadBuffer(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + idx));
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _size.x, _size.y, 0, 0, _size.x, _size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		return result;
	}

	std::shared_ptr<ITexture>& operator[](size_t idx) final {
		return _attachments[static_cast<GLuint>(idx)];
	}

private:
	GLuint _framebuffer;
	GLuint _resolverFramebuffer;
	std::map<GLuint, std::shared_ptr<ITexture>> _attachments;
	std::shared_ptr<ITexture> _depth;
	glm::ivec2 _size;
	size_t _samples;
};


std::shared_ptr<IFramebuffer> GLFramebuffer::create(glm::ivec2 size, size_t samples) {
	return std::shared_ptr<IFramebuffer>(new ::GLFramebufferImpl(size, samples));
}
