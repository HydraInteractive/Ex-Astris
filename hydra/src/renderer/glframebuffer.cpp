#include <hydra/renderer/glrenderer.hpp>

#include <hydra/engine.hpp>

#include <glad/glad.h>

#include <map>

using namespace Hydra::Renderer;

inline static GLenum toGL(FramebufferTextureType type) {
	static const GLenum translate[] = {
		GL_RED, // r
		GL_RG, // rg
		GL_RGB, // rgb
		GL_RGBA, // rgba

		GL_R16F, // f16R
		GL_RG16F, // f16RG
		GL_RGB16F, // f16RGB
		GL_RGBA16F, // f16RGBA
		GL_R32F, // f32R
		GL_RG32F, // f32RG
		GL_RGB32F, // f32RGB
		GL_RGBA32F, // f32RGBA

		GL_DEPTH_COMPONENT16, // depth16
		GL_DEPTH_COMPONENT32 // depth32
	};
	return translate[static_cast<int>(type)];
}

class GLFramebufferImpl final : public IFramebuffer {
public:
	GLFramebufferImpl(glm::ivec2 size, size_t samples) : _size(size), _samples(samples) {
		glGenFramebuffers(1, &_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	}

	~GLFramebufferImpl() final {
		glDeleteFramebuffers(1, &_framebuffer);
	}

	// ITexture
	glm::ivec2 getSize() final { return _size; }
	uint32_t getID() const final { return _framebuffer; }

	// IFramebuffer
	IFramebuffer& addTexture(size_t id, FramebufferTextureType type) final {
		//TODO: IF samples == 0, then don't use _MULTISAMPLE
		GLuint texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texID);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _samples, toGL(type), _size.x, _size.y, GL_FALSE);
		if (static_cast<uint32_t>(type) >= static_cast<uint32_t>(FramebufferTextureType::depth16))
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texID, 0);
		else
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id, texID, 0);

		_attachments[id] = GLTexture::createFromID(texID, _size);

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
	std::shared_ptr<ITexture> resolve(size_t idx) final {
		std::shared_ptr<ITexture> result = GLTexture::createFromData(_size.x, _size.y, nullptr);
		GLuint tmpFBO;
		// TODO: CLEAN UP!, maybe not force MSAA on FBO
		glGenFramebuffers(1, &tmpFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, tmpFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result->getID(), 0);
		GLenum buffers = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &buffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tmpFBO);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _size.x, _size.y, 0, 0, _size.x, _size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glDeleteFramebuffers(1, &tmpFBO);
		return result;
	}

	std::shared_ptr<ITexture> operator[](size_t idx) final {
		return _attachments[idx];
	}

private:
	GLuint _framebuffer;
	std::map<GLuint, std::shared_ptr<ITexture>> _attachments;
	glm::ivec2 _size;
	size_t _samples;
};


std::shared_ptr<IFramebuffer> GLFramebuffer::create(glm::ivec2 size, size_t samples) {
	return std::shared_ptr<IFramebuffer>(new ::GLFramebufferImpl(size, samples));
}
