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
	}

	glm::ivec2 getSize() final { return _size; }
	uint32_t getID() const final { return _framebuffer; }

	// IFramebuffer
	IFramebuffer& addTexture(size_t id, TextureType type) final {
		auto texture = GLTexture::createEmpty(_size.x, _size.y, type, _samples);

		if (static_cast<uint32_t>(type) >= static_cast<uint32_t>(TextureType::f16Depth))
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->getID(), 0);
		else
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id, texture->getID(), 0);

		_attachments[id] = texture;

		return *this;
	}

	void finalize() final {
		using namespace Hydra;
		std::vector<GLenum> buffers;
		for (auto it = _attachments.begin(); it != _attachments.end(); ++it)
			buffers.push_back(GL_COLOR_ATTACHMENT0 + it->first);
		glDrawBuffers(buffers.size(), &buffers[0]);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			IEngine::getInstance()->log(LogLevel::error, "Framebuffer failed! Status: %x", status);
	}

	//TODO: Add resolve to FBO and not just a texture
	std::shared_ptr<ITexture> resolve(size_t idx, std::shared_ptr<ITexture> result) final {
		result->resize(_size);

		glBindFramebuffer(GL_FRAMEBUFFER, _resolverFramebuffer);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result->getID(), 0);
		GLenum buffers = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &buffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolverFramebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + idx);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _size.x, _size.y, 0, 0, _size.x, _size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		return result;
	}

	std::shared_ptr<ITexture> operator[](size_t idx) final {
		return _attachments[idx];
	}

private:
	GLuint _framebuffer;
	GLuint _resolverFramebuffer;
	std::map<GLuint, std::shared_ptr<ITexture>> _attachments;
	glm::ivec2 _size;
	size_t _samples;
};


std::shared_ptr<IFramebuffer> GLFramebuffer::create(glm::ivec2 size, size_t samples) {
	return std::shared_ptr<IFramebuffer>(new ::GLFramebufferImpl(size, samples));
}
