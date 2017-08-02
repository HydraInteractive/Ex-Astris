#pragma once

#include <memory>

#include <hydra/renderer/renderer.hpp>
#include <hydra/world/world.hpp>
#include <hydra/io/textureloader.hpp>

namespace Hydra {
	class IEngine {
	public:
		virtual ~IEngine() = 0;

		virtual void run() = 0;

		virtual World::IWorld* getWorld() = 0;
		virtual Renderer::IRenderer* getRenderer() = 0;
		virtual IO::TextureLoader* getTextureLoader() = 0;
	};
	inline IEngine::~IEngine() {}
}
