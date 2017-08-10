#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace Hydra::Renderer;

class GLTextureImpl final : public ITexture {
public:
	GLTextureImpl(GLuint texture, glm::ivec2 size) : _own(false), _texture(texture), _size(size) { }

	GLTextureImpl(const std::string& file) : _own(true) {
		SDL_Surface* surface = IMG_Load(file.c_str());
		if (!surface)
			throw "Texture failed to load!";

		GLenum format;

		int nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGBA;
			else
				format = GL_BGRA;
		} else if (nOfColors == 3) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGB;
			else
				format = GL_BGR;
		} else {
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			format = GL_RGB;
		}

		_setData(format, surface->w, surface->h, surface->pixels);

		SDL_FreeSurface(surface);
	}

	GLTextureImpl(uint32_t width, uint32_t height, void* data) : _own(true) {
		_setData(GL_RGBA, width, height, data);
	}

	GLTextureImpl(const char* ext, void* data, uint32_t size) : _own(true) {
		SDL_Surface* surface = IMG_LoadTyped_RW(SDL_RWFromConstMem(data, size), 1, ext);
		if (!surface)
			throw "Texture failed to load!";

		GLenum format;

		int nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGBA;
			else
				format = GL_BGRA;
		} else if (nOfColors == 3) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGB;
			else
				format = GL_BGR;
		} else {
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			format = GL_RGB;
		}

		_setData(format, surface->w, surface->h, surface->pixels);

		SDL_FreeSurface(surface);
	}

	~GLTextureImpl() final {
		if (_own)
			glDeleteTextures(1, &_texture);
	}

	glm::ivec2 getSize() final { return _size; }
	uint32_t getID() const final { return _texture; }

private:
	bool _own;
	GLuint _texture;
	glm::ivec2 _size;

	void _setData(GLenum format, GLuint w, GLuint h, const void* pixels) {
		_size = glm::ivec2{w, w};

		glGenTextures(1, &_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);

		// TODO: be able to change this
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
	}
};

std::shared_ptr<ITexture> GLTexture::createFromFile(const std::string& file) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(file));
}

std::shared_ptr<ITexture> GLTexture::createFromData(uint32_t width, uint32_t height, void* data) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(width, height, data));
}

std::shared_ptr<ITexture> GLTexture::createFromDataExt(const char* ext, void* data, uint32_t size) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(ext, data, size));
}
