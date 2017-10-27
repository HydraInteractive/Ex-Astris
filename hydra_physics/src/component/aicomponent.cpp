// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* AIComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#include <hydra/component/aicomponent.hpp>
#include <imgui/imgui.h>
#include <hydra/component/drawobjectcomponent.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

AIComponent::~AIComponent() {
	
}

void AIComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "scale",{ _scale.x, _scale.y, _scale.z } },
		{ "behaviourType", (unsigned int)behaviour->type},
		{ "pathState", behaviour->state },
		{ "bossPhase", (int)bossPhase },
		{ "damage", _damage },
		{ "range", _range },
		{ "mapOffset",{ mapOffset.x, mapOffset.y, mapOffset.z } },
		{ "Original range", _originalRange }
	};

	for (size_t i = 0; i < 64; i++)
	{
		for (size_t j = 0; j < 64; j++)
		{
			json["map"][i][j] = _map[i][j];
		}
	}
}

void AIComponent::deserialize(nlohmann::json& json) {
	auto& scale = json["scale"];
	_scale = glm::vec3{ scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>() };

	auto& _mapOffset = json["mapOffset"];
	mapOffset = glm::vec3{ _mapOffset[0].get<float>(), _mapOffset[1].get<float>(), _mapOffset[2].get<float>() };

	Behaviour::Type behaviourType = (Behaviour::Type)json["behaviourType"].get<unsigned int>();
	switch (behaviourType)
	{
	case Behaviour::Type::ALIEN:
		behaviour = std::make_shared<AlienBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::ROBOT:
		behaviour = std::make_shared<RobotBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	case Behaviour::Type::ALIENBOSS:
		behaviour = std::make_shared<AlienBossBehaviour>(Hydra::World::World::getEntity(entityID));
		break;
	default:
		std::cout << "Invalid AI Behaviour Type" << std::endl;
		break;
	}
	behaviour->state = json["pathState"].get<int>();
	bossPhase = (BossPhase)json["bossPhase"].get<int>();
	_damage = json["damage"].get<int>();


	_range = json["range"].get<float>();
	_originalRange = json["Original range"].get<float>();

	for (size_t i = 0; i < 64; i++)
	{
		for (size_t j = 0; j < 64; j++)
		{
			_map[i][j] = json["map"][i][j].get<int>();
		}
	}
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void AIComponent::registerUI() {
	ImGui::InputInt("pathState", &_debugState);
	ImGui::Checkbox("isAtGoal", &_isAtGoal);
	ImGui::Checkbox("playerCanBeSeen", &_playerSeen);
	ImGui::InputFloat("range", &_range);
}

bool Hydra::Component::AIComponent::_checkLOS(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B)
{
	// New code, not optimal
	double x = B.x - A.x;
	double z = B.z - A.z;
	double len = std::sqrt((x*x) + (z*z));

	if (!len) // Player tile is same as target tile
		return true;

	double unitx = x / len;
	double unitz = z / len;

	x = A.x;
	z = A.z;
	for (double i = 1; i < len; i += 1)
	{
		if (levelmap[(int)x][(int)z] == 1)
		{
			return false;
		}

		x += unitx;
		z += unitz;
	}

	return true;

	// Old code
	/*bool steep = (fabs(B.z - A.z) > fabs(B.x - A.x));
	if (steep)
	{
		std::swap(A.x, A.z);
		std::swap(B.x, B.z);
	}

	if (A.x > B.x)
	{
		std::swap(A.x, B.x);
		std::swap(A.z, B.z);
	}

	float dx = B.x - A.x;
	float dz = fabs(B.z - A.z);

	float error = dx / 2.0f;
	int zStep = (A.z < B.z) ? 1 : -1;
	int z = (int)A.z;

	int maxX = (int)B.x;

	int x;
	for (x = (int)A.x; x < maxX; x++)
	{
		if (steep)
		{
			_map[x][z] = 3;
			if (levelmap[x][z] == 1) return false;
		}
		else
		{
			_map[x][z] = 3;
			if (levelmap[z][x] == 1) return false;
		}

		error -= dz;
		if (error < 0)
		{
			z += zStep;
			error += dx;
		}
	}

	return true;*/
}

std::shared_ptr<Hydra::World::Entity> AIComponent::getPlayerEntity()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}

