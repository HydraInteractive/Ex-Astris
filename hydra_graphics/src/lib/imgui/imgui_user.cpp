#include <imgui/imgui_user.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImGui {
	bool Tab(size_t index, const char* label, const char* tooltip, size_t* selected) {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 itemSpacing = style.ItemSpacing;
		ImVec4 color = style.Colors[ImGuiCol_Button];
		ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
		style.ItemSpacing.x = 1;

		if (index > 0)
			ImGui::SameLine();

		// push the style
		if (index == *selected) {
			style.Colors[ImGuiCol_Button] = colorActive;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorActive;
		}	else {
			style.Colors[ImGuiCol_Button] = color;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		}

		// Draw the button
		if (ImGui::Button(label))
			*selected = index;

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("%s", tooltip);
			ImGui::EndTooltip();
		}

		// Restore the style
		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		style.ItemSpacing = itemSpacing;

		return *selected == index;
	}

}
