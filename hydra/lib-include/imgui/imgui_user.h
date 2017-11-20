#pragma once

#include "imguizmo.h"

#include <cstdint>
#include <cstddef>

namespace ImGui {
	IMGUI_API bool Tab(size_t index, const char* label, const char* tooltip, size_t* selected);
}
