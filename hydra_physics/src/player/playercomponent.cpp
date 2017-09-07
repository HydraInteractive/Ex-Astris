// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Description of the component.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <player/playercomponent.hpp>

#include <imgui/imgui.h>

using namespace Hydra::World;
using namespace Hydra::Component;

PlayerComponent::PlayerComponent(IEntity* entity) : IComponent(entity) {

}

PlayerComponent::~PlayerComponent() { }

void PlayerComponent::tick(TickAction action) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.
	
}

msgpack::packer<msgpack::sbuffer>& PlayerComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	/*
	* // Optional code, not really needed
	*
	* o.pack_map(3); // The amount of variables to be written
	*
	* // The variables:
	* o.pack("a");
	* o.pack(_a);
	*
	* o.pack("b");
	* o.pack(_b);
	*
	* o.pack("c");
	* o.pack(_c);
	*/
	return o;
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void PlayerComponent::registerUI() {
	ImGui::InputInt("A", &_a);
	ImGui::Checkbox("B", &_b);
	ImGui::DragFloat("C", &_c);
}
