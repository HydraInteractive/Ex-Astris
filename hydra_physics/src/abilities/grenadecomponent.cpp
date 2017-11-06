// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Bullet stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/abilities/grenadecomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

GrenadeComponent::~GrenadeComponent() { }

void GrenadeComponent::serialize(nlohmann::json& json) const {
	json = {
	};
}

void GrenadeComponent::deserialize(nlohmann::json& json) {
}

void GrenadeComponent::registerUI() {
}
