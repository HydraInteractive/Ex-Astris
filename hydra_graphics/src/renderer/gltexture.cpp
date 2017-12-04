// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the ITexture, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace Hydra::Renderer;

static GLenum toGLBase(TextureType type) {
	static const GLenum translate[] = {
		/* [_(TextureType::u8R)] = */ GL_RED,
		/* [_(TextureType::u8RG)] = */ GL_RG,
		/* [_(TextureType::u8RGB)] = */ GL_RGB,
		/* [_(TextureType::u8RGBA)] = */ GL_RGBA,

		/* [_(TextureType::f16R)] = */ GL_RED,
		/* [_(TextureType::f16RG)] = */ GL_RG,
		/* [_(TextureType::f16RGB)] = */ GL_RGB,
		/* [_(TextureType::f16RGBA)] = */ GL_RGBA,

		/* [_(TextureType::f32R)] = */ GL_RED,
		/* [_(TextureType::f32RG)] = */ GL_RG,
		/* [_(TextureType::f32RGB)] = */ GL_RGB,
		/* [_(TextureType::f32RGBA)] = */ GL_RGBA,

		/* [_(TextureType::f16Depth)] = */ GL_DEPTH_COMPONENT,
		/* [_(TextureType::f24Depth)] = */ GL_DEPTH_COMPONENT,
		/* [_(TextureType::f32Depth)] = */ GL_DEPTH_COMPONENT
	};
	return translate[static_cast<int>(type)];
}

static GLenum toGLInternal(TextureType type) {
	static const GLenum translate[] = {
		/* [_(TextureType::u8R)] = */GL_RED,
		/* [_(TextureType::u8RG)] = */GL_RG,
		/* [_(TextureType::u8RGB)] = */GL_RGB,
		/* [_(TextureType::u8RGBA)] = */GL_RGBA,

		/* [_(TextureType::f16R)] = */GL_R16F,
		/* [_(TextureType::f16RG)] = */GL_RG16F,
		/* [_(TextureType::f16RGB)] = */GL_RGB16F,
		/* [_(TextureType::f16RGBA)] = */GL_RGBA16F,

		/* [_(TextureType::f32R)] = */GL_R32F,
		/* [_(TextureType::f32RG)] = */GL_RG32F,
		/* [_(TextureType::f32RGB)] = */GL_RGB32F,
		/* [_(TextureType::f32RGBA)] = */GL_RGBA32F,

		/* [_(TextureType::f16Depth)] = */GL_DEPTH_COMPONENT16,
		/* [_(TextureType::f24Depth)] = */GL_DEPTH_COMPONENT24,
		/* [_(TextureType::f32Depth)] = */GL_DEPTH_COMPONENT32
	};
	return translate[static_cast<int>(type)];
}

static GLenum toGLDataType(TextureType type) {
	static const GLenum translate[] = {
		/* [_(TextureType::u8R)] = */ GL_UNSIGNED_BYTE,
		/* [_(TextureType::u8RG)] = */ GL_UNSIGNED_BYTE,
		/* [_(TextureType::u8RGB)] = */ GL_UNSIGNED_BYTE,
		/* [_(TextureType::u8RGBA)] = */ GL_UNSIGNED_BYTE,

		/* [_(TextureType::f16R)] = */ GL_FLOAT,
		/* [_(TextureType::f16RG)] = */ GL_FLOAT,
		/* [_(TextureType::f16RGB)] = */ GL_FLOAT,
		/* [_(TextureType::f16RGBA)] = */ GL_FLOAT,

		/* [_(TextureType::f32R)] = */ GL_FLOAT,
		/* [_(TextureType::f32RG)] = */ GL_FLOAT,
		/* [_(TextureType::f32RGB)] = */ GL_FLOAT,
		/* [_(TextureType::f32RGBA)] = */ GL_FLOAT,

		/* [_(TextureType::f16Depth)] = */ GL_FLOAT,
		/* [_(TextureType::f32Depth)] = */ GL_FLOAT,
		/* [_(TextureType::f32Depth)] = */ GL_FLOAT
	};
	return translate[static_cast<int>(type)];
}

class GLTextureImpl : public ITexture {
public:
	GLTextureImpl(GLuint texture, glm::ivec2 size, TextureType format, size_t samples, bool own) : _own(own), _texture(texture), _format(format), _samples(samples), _size(size) {
		_textureType = samples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	GLTextureImpl(const std::string& file) : _textureType(GL_TEXTURE_2D), _own(true), _samples(0) {
		SDL_Surface* surface = IMG_Load(file.c_str());
		if (!surface)
			throw "Texture failed to load!";

		{
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			_format = TextureType::u8RGBA;
		}

		_size = glm::ivec2{surface->w, surface->h};

		_setData(surface->pixels);

		SDL_FreeSurface(surface);
	}

	GLTextureImpl(uint32_t width, uint32_t height, TextureType format, size_t samples) : _own(true), _format(format), _samples(samples), _size(glm::ivec2{width, height}) {
		_textureType = samples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		_setData(nullptr);
	}

	GLTextureImpl(uint32_t width, uint32_t height, TextureType format, void* data) : _textureType(GL_TEXTURE_2D), _own(true), _format(format), _samples(0), _size(glm::ivec2{width, height}) {
		_setData(data);
	}

	GLTextureImpl(const char* ext, void* data, uint32_t size) : _textureType(GL_TEXTURE_2D), _own(true), _samples(0) {
		SDL_Surface* surface = IMG_LoadTyped_RW(SDL_RWFromConstMem(data, size), 1, ext);
		if (!surface)
			throw "Texture failed to load!";

		{
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			_format = TextureType::u8RGBA;
		}

		_size = glm::ivec2{surface->w, surface->h};

		_setData(surface->pixels);

		SDL_FreeSurface(surface);
	}

	// This one is for createDataTexture
	GLTextureImpl(uint32_t width, uint32_t height, TextureType format) : _textureType(GL_TEXTURE_RECTANGLE), _own(true), _format(format), _samples(0), _size(glm::ivec2{width, height}) {
		_setData(nullptr);
	}

	~GLTextureImpl() final {
		if (_own)
			glDeleteTextures(1, &_texture);
	}

	void resize(glm::ivec2 size) final {
		if (!_own)
			return;
		_size = size;

		glBindTexture(_textureType, _texture);
		if (_textureType == GL_TEXTURE_2D || _textureType == GL_TEXTURE_RECTANGLE)
			glTexImage2D(_textureType, 0, toGLInternal(_format), size.x, size.y, 0, toGLBase(_format), toGLDataType(_format), nullptr);
		else if (_textureType == GL_TEXTURE_2D_MULTISAMPLE)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)_samples, toGLBase(_format), (GLsizei)_size.x, (GLsizei)_size.y, GL_FALSE);
	}

	void bind(size_t position) final {
		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + position));
		if (!_samples)
			glBindTexture(_textureType, _texture);
		else
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _texture);
	}

	size_t getSamples() final { return _samples; }
	glm::ivec2 getSize() final { return _size; }
	uint32_t getID() const final { return _texture; }
	void setRepeat() final {
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void setClamp() final {
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void setData(const glm::ivec2& offset, const glm::ivec2& size, const void* data) final {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(_textureType, _texture);
		//Fail safe, Me thinks
		if(data != NULL)
			glTexSubImage2D(_textureType, 0, offset.x, offset.y, size.x, size.y, toGLBase(_format), toGLDataType(_format), data);
	}

private:
	GLenum _textureType;
	bool _own;
	GLuint _texture;
	TextureType _format;
	size_t _samples;
	glm::ivec2 _size;
	void* _mapAddress;

	void _setData(const void* pixels) {
		glGenTextures(1, &_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(_textureType, _texture);

		if (_textureType == GL_TEXTURE_2D || _textureType == GL_TEXTURE_RECTANGLE)
			glTexImage2D(_textureType, 0, toGLInternal(_format), _size.x, _size.y, 0, toGLBase(_format), toGLDataType(_format), pixels);
		else if (_textureType == GL_TEXTURE_2D_MULTISAMPLE)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)_samples, toGLBase(_format), (GLsizei)_size.x, (GLsizei)_size.y, GL_FALSE);

		if (_textureType == GL_TEXTURE_2D && _size.x == 4) {
			glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} else if (_textureType == GL_TEXTURE_2D || _textureType == GL_TEXTURE_RECTANGLE) {
			glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			if (_format != TextureType::f16R) {
				glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}
};

std::shared_ptr<ITexture> GLTexture::createFromID(uint32_t id, glm::ivec2 size, TextureType format, size_t samples, bool own) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(id, size, format, samples, own));
}

std::shared_ptr<ITexture> GLTexture::createFromFile(const std::string& file) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(file));
}

std::shared_ptr<ITexture> GLTexture::createEmpty(uint32_t width, uint32_t height, TextureType format, size_t samples) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(width, height, format, samples));
}

std::shared_ptr<ITexture> GLTexture::createFromData(uint32_t width, uint32_t height, TextureType format, void* data) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(width, height, format, data));
}

std::shared_ptr<ITexture> GLTexture::createFromDataExt(const char* ext, void* data, uint32_t size) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(ext, data, size));
}


std::shared_ptr<ITexture> GLTexture::createDataTexture(uint32_t width, uint32_t height, TextureType format) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(width, height, format));
}
