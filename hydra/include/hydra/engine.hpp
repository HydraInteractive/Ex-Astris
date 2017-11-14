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
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/world/world.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textfactory.hpp>
#include <hydra/system/deadsystem.hpp>

#ifndef PRINTFARGS
#if defined(__clang__) || defined(__GNUC__)
#define PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define PRINTFARGS(FMT)
#endif
#endif

namespace Hydra::View { class IView; }

namespace Hydra {
	enum class HYDRA_BASE_API LogLevel {
		verbose,
		normal,
		warning,
		error
	};
	HYDRA_BASE_API inline const char* toString(LogLevel level) {
		static const char* name[4] = {"verbose", "normal", "warning", "error"};
		return name[static_cast<int>(level)];
	}

	class HYDRA_BASE_API IState {
	public:
		virtual ~IState() = 0;

		virtual void load() = 0;
		virtual void onMainMenu() = 0;

		/// Update and render a frame
		virtual void runFrame(float delta) = 0;

		virtual IO::ITextureLoader* getTextureLoader() = 0;
		virtual IO::IMeshLoader* getMeshLoader() = 0;
		virtual IO::ITextFactory* getTextFactory() = 0;
		virtual World::ISystem* getPhysicsSystem() = 0;
	};
	inline IState::~IState() {}

	class HYDRA_BASE_API IEngine {
	public:
		virtual ~IEngine() = 0;

		virtual void run() = 0;

		virtual void quit() = 0;

		virtual void onMainMenu() = 0;

		template <typename T, typename... Args, typename std::enable_if<std::is_base_of<IState, T>::value>::type* = nullptr>
		T* setState(Args... args) {
			T* ptr = new T(args...);
			setState_(std::unique_ptr<IState>(ptr));
			return ptr;
		}
		virtual void setState_(std::unique_ptr<IState> state) = 0;
		virtual IState* getState() = 0;
		virtual View::IView* getView() = 0;
		virtual Renderer::IRenderer* getRenderer() = 0;
		virtual Renderer::IUIRenderer* getUIRenderer() = 0;
		virtual Hydra::System::DeadSystem* getDeadSystem() = 0;

		virtual void log(LogLevel level, const char* fmt, ...) PRINTFARGS(3) = 0;

		static IEngine*& getInstance();

	private:
		static IEngine* _instance;
	};
	inline IEngine::~IEngine() {}
}
