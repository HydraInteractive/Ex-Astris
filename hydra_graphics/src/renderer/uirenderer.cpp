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
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <memory>
#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/ext/ram.hpp>
#include <hydra/ext/vram.hpp>

#include <hydra/component/transformcomponent.hpp>

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
		ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_Once);
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

class UIRendererImpl;
template <typename... Args>
struct RenderComponents;

template <>
struct RenderComponents<Hydra::Ext::TypeTuple<>> {
	constexpr static void apply(UIRendererImpl*, Entity*) {}
};

template <typename T, typename... Args>
struct RenderComponents<Hydra::Ext::TypeTuple<T, Args...>> {
	constexpr static void apply(UIRendererImpl* this_, Entity* entity);
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

		ImGui::StyleColorsDark();
	}

	~UIRendererImpl() final {
		ImGui_ImplSdlGL3_Shutdown();
	}

	void handleEvent(SDL_Event& event) final { ImGui_ImplSdlGL3_ProcessEvent(&event); }

	void newFrame() final {
		ImGui_ImplSdlGL3_NewFrame(_window);
		ImGuizmo::BeginFrame();
	}

	void reset() final {
		_systems.clear();
	}

	void registerSystems(const std::vector<Hydra::World::ISystem*>& systems) final {
		// TODO: Merge instead of replace?
		_systems = systems;
	}

	void render(float delta) final {
		//constexpr float MiB = 1024.0f * 1024.0f;
		//if (ImGui::BeginMainMenuBar()) {
		//	if (ImGui::BeginMenu("File")) {
		//		ImGui::MenuItem("Log", nullptr, &_logWindow);
		//		ImGui::MenuItem("Entity List", nullptr, &_entityWindow);
		//		ImGui::MenuItem("Performance Window", nullptr, &_performanceWindow);
		//		ImGui::Separator();
		//		ImGui::MenuItem("ImGui Test Window", nullptr, &_testWindow);
		//		ImGui::Separator();
		//		if (ImGui::MenuItem("Quit"))
		//				_view->quit();
		//		ImGui::EndMenu();
		//	}

		//	_engine->onMainMenu();

		//	static char buf[128];
		//	float fps = ImGui::GetIO().Framerate;

		//	const float ramReal = Hydra::Ext::getCurrentRSS() / MiB;
		//	const float ramPeak = Hydra::Ext::getPeakRSS() / MiB;
		//	const float vram = Hydra::Ext::getCurrentVRAM() / MiB;
		//	const float vramMax = Hydra::Ext::getMaxVRAM() / MiB;
		//	float ram = ramReal - vram;
		//	if (Hydra::Ext::isVRAMDedicated())
		//		ram = ramReal;
		//	snprintf(buf, sizeof(buf), "Application average %.3f ms/frame (%.1f FPS) - RAM: %.2fMiB (Real: %.2fMiB / Peak %.2fMiB) - VRAM: %.2fMiB / %.2fMiB", 1000.0f / fps, fps, ram, ramReal, ramPeak, vram, vramMax);

		//	auto indent = _view->getSize().x / 2 - ImGui::CalcTextSize(buf).x / 2;

		//	ImGui::Indent(indent);
		//	ImGui::Text("%s", buf);
		//	ImGui::Unindent(indent);

		//	ImGui::EndMainMenuBar();
		//}

		//{
		//	constexpr int valueLen = 128;
		//	static float fpsValues[valueLen] = {0};
		//	static float ramValues[valueLen] = {0};
		//	static float vramValues[valueLen] = {0};

		//	static char fpsName[64] = {0};
		//	static char ramName[64] = {0};
		//	static char vramName[64] = {0};

		//	static float counter = 0;

		//	counter += delta;

		//	if (counter >= 0.1f) {
		//		counter -= 0.1f;
		//		memmove(&fpsValues[0], &fpsValues[1], (valueLen - 1) * sizeof(float));
		//		fpsValues[valueLen - 1] = ImGui::GetIO().Framerate;
		//		snprintf(fpsName, sizeof(fpsName), "FPS: %.2f / %.3f ms/frame", fpsValues[valueLen - 1], 1000.0f / fpsValues[valueLen - 1]);

		//		memmove(&ramValues[0], &ramValues[1], (valueLen - 1) * sizeof(float));
		//		ramValues[valueLen - 1] = (Hydra::Ext::getCurrentRSS() - Hydra::Ext::getCurrentVRAM()) / MiB;
		//		snprintf(ramName, sizeof(ramName), "RAM: %.2f MiB", ramValues[valueLen - 1]);

		//		memmove(&vramValues[0], &vramValues[1], (valueLen - 1) * sizeof(float));
		//		vramValues[valueLen - 1] = Hydra::Ext::getCurrentVRAM() / MiB;
		//		snprintf(vramName, sizeof(vramName), "VRAM: %.2f MiB", vramValues[valueLen - 1]);
		//	}

		//	if (_performanceWindow) {
		//		ImGui::SetNextWindowPos(ImVec2(_view->getSize().x - (300 + 16), 24), ImGuiCond_Always);
		//		//ImGui::SetNextWindowSize(ImVec2(300 + 16, 300 + 24), ImGuiCond_Always);
		//		ImGui::Begin("Performance monitor", &_performanceWindow, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
		//		//ImGui::Text("Counter: %f", counter);
		//		ImGui::PlotHistogram("##FPS", fpsValues, valueLen, 0, fpsName, FLT_MAX, FLT_MAX, ImVec2(300, 100));
		//		ImGui::PlotHistogram("##RAM", ramValues, valueLen, 0, ramName, FLT_MAX, FLT_MAX, ImVec2(300, 100));
		//		ImGui::PlotHistogram("##VRAM", vramValues, valueLen, 0, vramName, FLT_MAX, FLT_MAX, ImVec2(300, 100));
		//		ImGui::End();
		//	}
		//}

		//_engine->getRenderer()->showGuizmo();

		//if (_logWindow)
		//	_log->render(&_logWindow);

		//if (_entityWindow)
		//	_renderEntityWindow();

		//if (_testWindow)
		//	ImGui::ShowTestWindow(&_testWindow);

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

	bool isDraging() final { return ImGui::IsMouseDragging() || ImGuizmo::IsUsing(); }

	void renderEntity(Entity* entity) {
		if (!entity)
			return;
		using world = Hydra::World::World;
		if (ImGui::TreeNode(entity, ICON_FA_USER_O " %s [%lu] ( " ICON_FA_MICROCHIP " %lu / " ICON_FA_USER_O " %lu )", entity->name.c_str(), entity->id, entity->componentCount(), entity->children.size()))
		{
			std::string entityID = std::to_string(entity->id);
			if (ImGui::BeginPopupContextItem(entityID.c_str()))
			{				
				if (ImGui::MenuItem("New Child..."))
				{
					newEntityParent = entity->id;
				}
				if (ImGui::MenuItem("Delete"))
				{
					world::removeEntity(entity->id);
				}
				ImGui::EndPopup();
			}
			RenderComponents<Hydra::Component::ComponentTypes>::apply(this, entity);

			for (auto& child : entity->children)
				renderEntity(world::getEntity(child).get());
			ImGui::TreePop();
		}
		else
		{
			std::string entityID = std::to_string(entity->id);
			if (ImGui::BeginPopupContextItem(entityID.c_str()))
			{
				if (ImGui::MenuItem("New Child..."))
				{
					newEntityParent = entity->id;
				}
				if (ImGui::MenuItem("Delete"))
				{
					world::removeEntity(entity->id);
				}
				ImGui::EndPopup();
			}
		}

	}

	void renderComponent(IComponentBase* component) {
		if (!component)
			return;
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
		if (!ImGui::TreeNodeEx(component, nodeFlags, ICON_FA_MICROCHIP " %s", component->type().c_str()))
			return;

		component->registerUI();
		ImGui::TreePop();
	}

private:
	Hydra::IEngine* _engine;
	Hydra::View::IView* _view;
	SDL_Window* _window;
	std::unique_ptr<IUILog> _log;

	std::vector<Hydra::World::ISystem*> _systems;

	bool _logWindow = false;
	bool _entityWindow = false;
	bool _performanceWindow = false;
	bool _testWindow = false;

	ImFont* _normalFont;
	ImFont* _normalBoldFont;
	ImFont* _mediumFont;
	ImFont* _monospaceFont;
	ImFont* _bigFont;

	int newEntityParent = -1;
	char inputText[32] = "";

	void _renderEntityWindow() {
		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_Once);
		ImGui::Begin("Entity List", &_entityWindow);

		using world = Hydra::World::World;
		auto worldRoot = world::root().get();

		// This doesn't use _renderEntity, because I want a globe instad of a user
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx(worldRoot, nodeFlags, ICON_FA_GLOBE " %s [%lu] ( " ICON_FA_MICROCHIP " %lu / " ICON_FA_USER_O " %lu )", worldRoot->name.c_str(), worldRoot->id, worldRoot->componentCount(), worldRoot->children.size())) {
			RenderComponents<Hydra::Component::ComponentTypes>::apply(this, worldRoot);

			std::string entityID = std::to_string(worldRoot->id);
			if (ImGui::BeginPopupContextItem(entityID.c_str()))
			{
				if (ImGui::MenuItem("New Child..."))
				{
					newEntityParent = worldRoot->id;
				}
				ImGui::EndPopup();
			}

			for (auto& child : worldRoot->children)
				renderEntity(world::getEntity(child).get());
			ImGui::TreePop();
		}
		else
		{
			std::string entityID = std::to_string(worldRoot->id);
			if (ImGui::BeginPopupContextItem(entityID.c_str()))
			{
				if (ImGui::MenuItem("New Child..."))
				{
					newEntityParent = worldRoot->id;
				}
				ImGui::EndPopup();
			}
		}
		//Open new entity popup
		if (newEntityParent != -1)
		{
			ImGui::OpenPopup("New Entity");
		}
		if (ImGui::BeginPopupModal("New Entity", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("Enter name of new entity: ", inputText, 32);
			ImGui::Separator();

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				std::string cstrToStr = inputText;
				world::newEntity(cstrToStr, newEntityParent);
				newEntityParent = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				newEntityParent = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::End();
	}
};

/*template <typename T, typename... Args>
struct RenderComponents<Hydra::Ext::TypeTuple<T, Args...>>:: {
	constexpr static void apply(UIRendererImpl* this_, Entity* entity) {
		this_->_renderComponentWrapper<T>(entity);
		RenderComponents<Hydra::Ext::TypeTuple<Args...>>::apply(this_, entity);
	}
	};*/


template <typename T, typename... Args>
constexpr void RenderComponents<Hydra::Ext::TypeTuple<T, Args...>>::apply(UIRendererImpl* this_, Entity* entity) {
	if (auto t = entity->getComponent<T>())
		this_->renderComponent(t.get());
	RenderComponents<Hydra::Ext::TypeTuple<Args...>>::apply(this_, entity);
}

std::unique_ptr<IUIRenderer> UIRenderer::create(Hydra::View::IView& view) {
	return std::unique_ptr<IUIRenderer>(new ::UIRendererImpl(view));
}
