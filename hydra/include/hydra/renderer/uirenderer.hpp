#pragma once

#include <hydra/view/view.hpp>
#include <memory>
#include <cstdarg>

// TODO: Abstact away?
union SDL_Event;

#ifndef PRINTFARGS
#if defined(__clang__) || defined(__GNUC__)
#define PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define PRINTFARGS(FMT)
#endif
#endif

namespace Hydra { enum class LogLevel; }

namespace Hydra::Renderer {
	class IUILog;

	enum class UIFont {
		normal,
		medium,
		big,
		monospace
	};

	class IUIRenderer {
	public:
		virtual ~IUIRenderer() = 0;

		virtual void handleEvent(SDL_Event& event) = 0;

		virtual void newFrame() = 0;

		virtual void render() = 0; // TODO: Move to IRenderer(?)

		virtual void pushFont(UIFont font) = 0;
		virtual void popFont() = 0;

		virtual IUILog* getLog() = 0;

		virtual bool usingKeyboard() = 0;
	};
	inline IUIRenderer::~IUIRenderer() {}

	class IUILog {
	public:
		virtual ~IUILog() = 0;

		void log(Hydra::LogLevel level, const char* fmt, ...) PRINTFARGS(3) {
			va_list va;
			va_start(va, fmt);
			log(level, fmt, va);
			va_end(va);
		}

		virtual void log(Hydra::LogLevel level, const char* fmt, va_list va) = 0;

		virtual void clear() = 0;

		virtual void render(bool* pOpen) = 0;
	};
	inline IUILog::~IUILog() {}

	namespace UIRenderer {
		std::unique_ptr<IUIRenderer> create(Hydra::View::IView& view);
	};
};
