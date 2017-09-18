// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A wrapper around Dear ImGui to be able to create debug UIs.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/uirenderer.hpp>

#include <imgui/imgui.h>
#include <imgui/icons.hpp>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <memory>
#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/ext/ram.hpp>

#include <iomanip>
#include <sstream>

using namespace Hydra::Renderer;
using namespace Hydra::World;

class UILogImpl final : public IUILog {
public:
	UILogImpl(IUIRenderer* uiRenderer) : _uiRenderer(uiRenderer), _scrollToBottom(false) {
		_buffer.append("Log initialized");
		_lineInfo.push_back(Line{0, (size_t)_buffer.size() - 1, Hydra::LogLevel::normal, 1});
	}
	~UILogImpl() final {}

	void log(Hydra::LogLevel level, const char* fmt, va_list args) final {
		static char tmp[0x1000];

		vsnprintf(tmp, sizeof(tmp), fmt, args);

		auto& lastLine = _lineInfo.back();
		if (std::string(&(_buffer.c_str()[lastLine.start]), lastLine.end - lastLine.start + 1) == std::string(tmp)) {
			lastLine.count++;
			return;
		}

		size_t oldSize = _buffer.size();
		_buffer.append("%s", tmp);

		_lineInfo.push_back(Line{oldSize, (size_t)_buffer.size() - 1, level, 1});
		_scrollToBottom = true;
	}

	void clear() final {
		_buffer.clear();
		_lineInfo.clear();
	}

	void render(bool* pOpen) final {
		ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiSetCond_Once);
		ImGui::Begin("Hydra Log", pOpen);
		if (ImGui::Button("Clear"))
			clear();
		ImGui::SameLine();

		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		_filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy)
			ImGui::LogToClipboard();

		_uiRenderer->pushFont(UIFont::monospace);

		const char * fmt = "[Count:%06d Level:%s]";

		if (_filter.IsActive()) {
			const char* bufBegin = _buffer.begin();
			for (size_t i = 0; i < _lineInfo.size(); i++) {
				const char* line = bufBegin + _lineInfo[i].start;
				const char* lineEnd = bufBegin + _lineInfo[i].end + 1;
				if (_filter.PassFilter(line, lineEnd)) {
					ImGui::Text(fmt, _lineInfo[i].count, Hydra::toString(_lineInfo[i].level));
					ImGui::SameLine();
					ImGui::TextUnformatted(line, lineEnd);
				}
			}
		} else {
			const char* bufBegin = _buffer.begin();
			const int maxLines = 4096;
			for (size_t i = std::max(0, (int)_lineInfo.size() - maxLines); i < _lineInfo.size(); i++) {
				const char* line = bufBegin + _lineInfo[i].start;
				const char* lineEnd = bufBegin + _lineInfo[i].end + 1;
				ImGui::Text(fmt, _lineInfo[i].count, Hydra::toString(_lineInfo[i].level));
				ImGui::SameLine();
				ImGui::TextUnformatted(line, lineEnd);
			}
		}
		_uiRenderer->popFont();

		if (_scrollToBottom)
			ImGui::SetScrollHere(1.0f);
		_scrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}

private:
	struct Line {
		size_t start;
		size_t end;
		Hydra::LogLevel level;
		int count;
	};

	IUIRenderer* _uiRenderer;

	ImGuiTextBuffer _buffer;
	ImGuiTextFilter _filter;
	std::vector<Line> _lineInfo;
	bool _scrollToBottom;
};

class UIRendererImpl final : public IUIRenderer {
public:
	UIRendererImpl(Hydra::View::IView& view) : _engine(Hydra::IEngine::getInstance()), _view(&view) {
    ImGui_ImplSdlGL3_Init(_window = static_cast<SDL_Window*>(view.getHandler()));

		_log = std::unique_ptr<IUILog>(new UILogImpl(this));

		ImGuiIO& io = ImGui::GetIO();
		_normalFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans.ttf", 18.0f);
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("assets/fonts/fontawesome-webfont.ttf", 18.0f, &icons_config, icons_ranges);

		_normalBoldFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans-Bold.ttf", 18.0f);
		_mediumFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans-Bold.ttf", 32);
		_bigFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans-Bold.ttf", 64 + 32);

		_monospaceFont = io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Regular.ttf", 18.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.83f, 0.95f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.39f, 0.80f, 0.80f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.27f, 0.27f, 0.87f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.75f, 0.75f, 1.00f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.55f, 0.55f, 0.67f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.75f, 0.75f, 0.67f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.49f, 0.49f, 0.45f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.49f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.49f, 0.49f, 0.60f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.56f, 0.56f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.47f, 0.47f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.33f, 0.33f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.14f, 0.42f, 0.42f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33f, 0.78f, 0.78f, 0.67f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_ComboBg] = ImVec4(0.18f, 0.55f, 0.55f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.44f, 0.81f, 0.81f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.33f, 0.78f, 0.78f, 0.60f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.55f, 0.55f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.33f, 0.78f, 0.78f, 0.67f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.44f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.33f, 0.78f, 0.78f, 0.67f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.25f, 0.74f, 0.74f, 0.40f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.25f, 0.74f, 0.74f, 0.60f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.25f, 0.74f, 0.74f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.84f, 0.84f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.53f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.22f, 0.67f, 0.67f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.53f, 0.84f, 0.84f, 0.40f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.53f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.53f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.53f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.84f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.13f, 0.40f, 0.40f, 1.00f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.09f, 0.27f, 0.27f, 0.67f);
	}

	~UIRendererImpl() final {
		ImGui_ImplSdlGL3_Shutdown();
	}

	void handleEvent(SDL_Event& event) final { ImGui_ImplSdlGL3_ProcessEvent(&event); }

	void newFrame() final {
		ImGui_ImplSdlGL3_NewFrame(_window);
	}

	void reset() final {
		_renderWindows.clear();
	}

	UIRenderWindow* addRenderWindow() final {
		auto window = std::make_unique<UIRenderWindow>();
		UIRenderWindow* output = window.get();
		_renderWindows.push_back(std::move(window));
		return output;
	}

	void render() final {
    if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("Log", nullptr, &_logWindow);
				ImGui::MenuItem("Entity List", nullptr, &_entityWindow);
				ImGui::Separator();
				ImGui::MenuItem("ImGui Test Window", nullptr, &_testWindow);

				ImGui::Separator();

				if (ImGui::MenuItem("Quit"))
						_view->quit();
				ImGui::EndMenu();
			}

			_engine->onMainMenu();

			static char buf[128];
			snprintf(buf, sizeof(buf), "Application average %.3f ms/frame (%.1f FPS) - RAM: %.2fMiB / Peak %.2fMiB", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, Hydra::Ext::getCurrentRSS()/(1024.0f*1024.0f), Hydra::Ext::getPeakRSS()/(1024.0f*1024.0f));

			auto indent = _view->getSize().x / 2 - ImGui::CalcTextSize(buf).x / 2;

			ImGui::Indent(indent);
			ImGui::Text("%s", buf);
			ImGui::Unindent(indent);

			ImGui::EndMainMenuBar();
		}

		{ //TODO: Revert this, into multiple windows
			ImGui::Begin("Render Windows");
			ImGuiWindow* wind = ImGui::GetCurrentWindow();
			ImGuiStyle& style = ImGui::GetStyle();
			if (_renderWindows.size()) {
				pushFont(UIFont::normalBold);
				ImGui::BeginTabBar("#RenderWindows");
				popFont();
				ImGui::DrawTabsBackground();
				for (auto& window : _renderWindows) {
					if (!window->enabled)
						continue;

					if (!ImGui::AddTab(window->title.c_str()))
						continue;

					auto iSize = wind->Size - style.WindowPadding - ImVec2(24, 72);
					if (iSize.x <= 2) iSize.x = 2;
					if (iSize.y <= 2) iSize.y = 2;
					window->size = glm::ivec2{iSize.x, iSize.y};
					ImGui::Image(reinterpret_cast<ImTextureID>((size_t)window->image->getID()), iSize);
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}

		if (_logWindow)
			_log->render(&_logWindow);

		if (_entityWindow)
			_renderEntityWindow();

		if (_testWindow)
			ImGui::ShowTestWindow(&_testWindow);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the screen (0)
		ImGui::Render();
	}

	void pushFont(UIFont font) final {
		ImFont* lookup[] = {
			/*[_(UIFont::normal)] = */ _normalFont,
			/*[_(UIFont::normalBold)] = */_normalBoldFont,
			/*[_(UIFont::medium)] = */_mediumFont,
			/*[_(UIFont::big)] = */_bigFont,
			/*[_(UIFont::monospace)] = */_monospaceFont
		};
		ImGui::PushFont(lookup[static_cast<int>(font)]);
	}
	void popFont() final { ImGui::PopFont(); }

	IUILog* getLog() final { return _log.get(); }

	bool usingKeyboard() final { return ImGui::GetIO().WantCaptureKeyboard; }

	bool isDraging() final { return ImGui::IsMouseDragging(); }

private:
	Hydra::IEngine* _engine;
	Hydra::View::IView* _view;
	SDL_Window* _window;
	std::unique_ptr<IUILog> _log;

	std::vector<std::unique_ptr<UIRenderWindow>> _renderWindows;

	bool _logWindow = true;
	bool _entityWindow = true;
	bool _testWindow = false;

	ImFont* _normalFont;
	ImFont* _normalBoldFont;
	ImFont* _mediumFont;
	ImFont* _monospaceFont;
	ImFont* _bigFont;

	void _renderEntityWindow() {
		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiSetCond_Once);
		ImGui::Begin("Entity List", &_entityWindow);

		auto world = _engine->getState()->getWorld()->getWorldRoot().get();

		// This doesn't use _renderEntity, because I want a globe instad of a user
		if (ImGui::TreeNode(world, ICON_FA_GLOBE " %s [%lu] ( " ICON_FA_MICROCHIP " %lu / " ICON_FA_USER_O " %lu )", world->getName().c_str(), world->getID(), world->getComponents().size(), world->getChildren().size())) {
			for (auto& component : world->getComponents())
				_renderComponent(component.second.get());

			for (auto& child : world->getChildren())
				_renderEntity(child.get());
			ImGui::TreePop();
		}

		ImGui::End();
	}

	void _renderEntity(IEntity* entity) {
		if (!ImGui::TreeNode(entity, ICON_FA_USER_O " %s [%lu] ( " ICON_FA_MICROCHIP " %lu / " ICON_FA_USER_O " %lu )", entity->getName().c_str(), entity->getID(), entity->getComponents().size(), entity->getChildren().size()))
			return;

		for (auto& component : entity->getComponents())
			_renderComponent(component.second.get());
		for (auto& child : entity->getChildren())
			_renderEntity(child.get());
		ImGui::TreePop();
	}

	void _renderComponent(IComponent* component) {
		if (!ImGui::TreeNode(component, ICON_FA_MICROCHIP " %s", component->type().c_str()))
			return;

		component->registerUI();
		ImGui::TreePop();
	}
};

std::unique_ptr<IUIRenderer> UIRenderer::create(Hydra::View::IView& view) {
	return std::unique_ptr<IUIRenderer>(new ::UIRendererImpl(view));
}
