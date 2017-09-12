/**
 * Everything related to engine, as an interfaces.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

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
	enum class HYDRA_API LogLevel {
		verbose,
		normal,
		warning,
		error
	};
	HYDRA_API inline const char* toString(LogLevel level) {
		static const char* name[4] = {"verbose", "normal", "warning", "error"};
		return name[static_cast<int>(level)];
	}

	class HYDRA_API IState {
	public:
		virtual ~IState() = 0;

		/// Load data
		virtual void onEnter(IState* oldState) = 0;

		/// Update and render a frame
		virtual void runFrame() = 0;

		/// Unload data
		virtual void onLeave(IState* newState) = 0;

		virtual World::IWorld* getWorld() = 0;
	};
	inline IState::~IState() final {}

	class HYDRA_API IEngine {
	public:
		virtual ~IEngine() = 0;

		virtual void run() = 0;

		virtual IState* getState() = 0;
		virtual Renderer::IRenderer* getRenderer() = 0;
		virtual IO::ITextureLoader* getTextureLoader() = 0;
		virtual IO::IMeshLoader* getMeshLoader() = 0;

		virtual void log(LogLevel level, const char* fmt, ...) PRINTFARGS(3) = 0;

		static IEngine*& getInstance();

	private:
		static IEngine* _instance;
	};
	inline IEngine::~IEngine() {}
}
