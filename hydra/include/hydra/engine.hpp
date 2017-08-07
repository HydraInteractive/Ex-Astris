#pragma once

#include <memory>

#include <hydra/renderer/renderer.hpp>
#include <hydra/world/world.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/io/meshloader.hpp>

#ifndef PRINTFARGS
#if defined(__clang__) || defined(__GNUC__)
#define PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define PRINTFARGS(FMT)
#endif
#endif

namespace Hydra {
	enum class LogLevel {
		verbose,
		normal,
		warning,
		error
	};
	inline const char* toString(LogLevel level) {
		static const char* name[4] = {"verbose", "normal", "warning", "error"};
		return name[static_cast<int>(level)];
	}

	class IEngine {
	public:
		virtual ~IEngine() = 0;

		virtual void run() = 0;

		virtual World::IWorld* getWorld() = 0;
		virtual Renderer::IRenderer* getRenderer() = 0;
		virtual IO::TextureLoader* getTextureLoader() = 0;
		virtual IO::MeshLoader* getMeshLoader() = 0;

		virtual void log(LogLevel level, const char* fmt, ...) PRINTFARGS(3) = 0;

		inline static IEngine*& getInstance() { return _instance; }

	private:
		static IEngine* _instance;
	};
	inline IEngine::~IEngine() {}
}
