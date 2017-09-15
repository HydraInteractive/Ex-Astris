// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Description of the component.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */

#include <hydra/component/NAMEcomponent.hpp>

#include <imgui/imgui.h>

using namespace Hydra::World;
using namespace Hydra::Component;

NAMEComponent::NAMEComponent(IEntity* entity) : IComponent(entity), _c(0) { }

NAMEComponent::NAMEComponent(IEntity* entity, int a, bool b, float c) : IComponent(entity), _a(a), _b(b), _c(c) { }

NAMEComponent::~NAMEComponent() final { }

void NAMEComponent::tick(TickAction action) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.
}

void MeshComponent::serialize(nlohmann::json& json) const {
	json = {
		{"a", _a},
		{"b", _b},
		{"c", _c}
	};
}

void MeshComponent::deserialize(nlohmann::json& json) {
	_a = json["a"].get<int>();
	_b = json["b"].get<bool>();
	_c = json["c"].get<float>();
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void NAMEComponent::registerUI() {
	ImGui::InputInt("A", &_a);
	ImGui::Checkbox("B", &_b);
	ImGui::DragFloat("C", &_c);
}
