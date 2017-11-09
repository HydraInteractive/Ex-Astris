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

Hydra::Component::LifeComponent::~LifeComponent()
{
}

void Hydra::Component::LifeComponent::applyDamage(int damage)
{
	if (health > 0)
	{
		health -= damage;
	}
}

bool Hydra::Component::LifeComponent::statusCheck()
{
	if (health <= 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Hydra::Component::LifeComponent::serialize(nlohmann::json & json) const
{
	json = {
		{ "maxHP", maxHP },
		{ "health", health },
	};
}

void Hydra::Component::LifeComponent::deserialize(nlohmann::json & json)
{
	maxHP = json["maxHP"].get<float>();
	health = json["health"].get<float>();
}

void Hydra::Component::LifeComponent::registerUI()
{
	ImGui::InputFloat("maxHP", &maxHP);
	ImGui::InputFloat("health", &health);
}
