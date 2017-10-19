// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* EnemyComponent/AI.
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

//void EnemyComponent::init(EnemyTypes enemyID, glm::vec3 pos, int hp, int dmg, float range, glm::vec3 scale) {
//	_enemyID = enemyID;
//	_position = pos;
//	_startPosition = pos;
//	_health = hp;
//	_damage = dmg;
//	_range = range;
//	_originalRange = range;
//	_scale = scale;
//
//	_attackTimer = SDL_GetTicks();
//	_spawnTimer = SDL_GetTicks();
//	_stunTimer = SDL_GetTicks();
//
//	_mapOffset = glm::vec3(-30.0f, 0, -30.0f);
//
//	for (int i = 0; i < 3; i++)
//	{
//		_map[10][10 + i] = 1;
//	}
//}

EnemyComponent::~EnemyComponent() {
	delete _pathFinding;
}

void EnemyComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "startPosition",{ _startPosition.x, _startPosition.y, _startPosition.z } },
		{ "scale",{ _scale.x, _scale.y, _scale.z } },
		{ "mapOffset",{ _mapOffset.x, _mapOffset.y, _mapOffset.z } },
		{ "velocity",{ _velocity.x, _velocity.y, _velocity.z } },
		{ "enemyID", (int)_enemyID },
		{ "pathState", (int)_pathState },
		{ "bossPhase", (int)_bossPhase },
		{ "damage", _damage },
		{ "range", _range },
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

void EnemyComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& startpos = json["startPosition"];
	_startPosition = glm::vec3{ startpos[0].get<float>(), startpos[1].get<float>(), startpos[2].get<float>() };

	auto& scale = json["scale"];
	_scale = glm::vec3{ scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>() };

	auto& mapOffset = json["mapOffset"];
	_mapOffset = glm::vec3{ mapOffset[0].get<float>(), mapOffset[1].get<float>(), mapOffset[2].get<float>() };

	auto& velocity = json["velocity"];
	_velocity = glm::vec3{ velocity[0].get<float>(), velocity[1].get<float>(), velocity[2].get<float>() };

	_range = json["range"].get<float>();
	_originalRange = json["Original range"].get<float>();

	_enemyID = (EnemyTypes)json["enemyID"].get<int>();
	_pathState = (PathState)json["pathState"].get<int>();
	_bossPhase = (BossPhase)json["bossPhase"].get<int>();
	_damage = json["damage"].get<int>();

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
void EnemyComponent::registerUI() {
	ImGui::InputFloat("X", &_position.x);
	ImGui::InputFloat("Y", &_position.y);
	ImGui::InputFloat("Z", &_position.z);
	ImGui::InputFloat("startY", &_startPosition.y);
	ImGui::InputInt("pathState", &_debugState);
	ImGui::InputFloat("targetX", &_targetPos.x);
	ImGui::InputFloat("targetY", &_targetPos.y);
	ImGui::InputFloat("targetZ", &_targetPos.z);
	ImGui::Checkbox("isAtGoal", &_isAtGoal);
	ImGui::Checkbox("playerCanBeSeen", &_playerSeen);
	ImGui::InputFloat("range", &_range);
}

bool Hydra::Component::EnemyComponent::_checkLOS(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B)
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

std::shared_ptr<Hydra::World::Entity> EnemyComponent::getPlayerComponent()
{
	return Hydra::World::World::getEntity(PlayerComponent::componentHandler->getActiveComponents()[0]->entityID);
}

