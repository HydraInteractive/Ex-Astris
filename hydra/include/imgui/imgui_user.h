#pragma once

#include "imgui_tabs.h"

#include <cstdint>

namespace ImGui {
	IMGUI_API bool Tab(size_t index, const char* label, const char* tooltip, size_t* selected);
}
