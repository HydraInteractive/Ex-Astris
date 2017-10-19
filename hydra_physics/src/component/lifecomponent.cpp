#include "hydra\component\lifecomponent.hpp"
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
		health = -damage;
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
	maxHP = json["maxHP"].get<int>();
	health = json["health"].get<int>();
}

void Hydra::Component::LifeComponent::registerUI()
{
	ImGui::InputInt("maxHP", &maxHP);
	ImGui::InputInt("health", &health);
}
