#include <hydra/renderer/uirenderer.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <SDL/SDL.h>

using namespace Hydra::Renderer;

class UIRendererImpl final : public IUIRenderer {
public:
	UIRendererImpl(Hydra::View::IView& view) : _view(&view) {
    ImGui_ImplSdlGL3_Init(_window = static_cast<SDL_Window*>(view.getHandler()));

		ImGuiIO& io = ImGui::GetIO();
		_defaultFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans.ttf", 18.0f);
		//_bigFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans-Bold.ttf", 64 + 32);
		//_mediumFont = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans-Bold.ttf", 32);

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

    if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("Test window", nullptr, &_testWindow);

				ImGui::Separator();

				if (ImGui::MenuItem("Quit"))
						_view->quit();
				ImGui::EndMenu();
			}

			static char buf[64];
			snprintf(buf, sizeof(buf), "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			auto indent = _view->getSize().x / 2 - ImGui::CalcTextSize(buf).x / 2 - /* File */4 / 2;

			ImGui::Indent(indent);
			ImGui::Text(buf);
			ImGui::Unindent(indent);

			ImGui::EndMainMenuBar();
		}

		if (_testWindow)
			ImGui::ShowTestWindow(&_testWindow);
	}

	void render() final { ImGui::Render(); }

	bool usingKeyboard() final { return ImGui::GetIO().WantCaptureKeyboard; }

private:
	Hydra::View::IView* _view;
	SDL_Window* _window;

	bool _testWindow = false;

	ImFont* _defaultFont;
	//ImFont* _bigFont;
	//ImFont* _mediumFont;
};

std::unique_ptr<IUIRenderer> UIRenderer::create(Hydra::View::IView& view) {
	return std::unique_ptr<IUIRenderer>(new ::UIRendererImpl(view));
}
