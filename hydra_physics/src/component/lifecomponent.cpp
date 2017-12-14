// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* LifeComponent used by living people.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/lifecomponent.hpp>
#include <imgui/imgui.h>

using namespace Hydra::World;
using namespace Hydra::Component;

Hydra::Component::LifeComponent::~LifeComponent() { }

void Hydra::Component::LifeComponent::applyDamage(int damage) {
	health -= damage;
}

bool Hydra::Component::LifeComponent::statusCheck() {
	return health > 0;
}

void Hydra::Component::LifeComponent::serialize(nlohmann::json& json) const {
	json["maxHP"] = maxHP;
	json["health"] = health;
	json["tickDownWithTime"] = tickDownWithTime;
}

void Hydra::Component::LifeComponent::deserialize(nlohmann::json& json) {
	maxHP = json.value<int>("maxHP", 0);
	health = json.value<int>("health", 0);
	tickDownWithTime = json.value<bool>("tickDownWithTime", false);
}

void Hydra::Component::LifeComponent::registerUI() {
	ImGui::InputFloat("maxHP", &maxHP);
	ImGui::InputFloat("health", &health);
	ImGui::Checkbox("Tick down with time", &tickDownWithTime);
}
