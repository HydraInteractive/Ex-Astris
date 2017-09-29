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

using namespace Hydra::World;
using namespace Hydra::Component;

EnemyComponent::EnemyComponent(IEntity* entity) : IComponent(entity), _enemyID(EnemyTypes::Alien), _position(0,0,0) {}

EnemyComponent::EnemyComponent(IEntity* entity, EnemyTypes enemyID, glm::vec3 pos) : IComponent(entity), _enemyID(enemyID),  _position(pos){
	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;
	_startPosition = glm::vec3(0, 0, 0);
	_patrolPointReached = false;
	_falling = false;
	_pathState = SEARCHING;
	lastTime = 0;

	for (int i = 0; i < WORLD_SIZE; i++)
	{
		for (int j = 0; j < WORLD_SIZE; j++)
		{
			map[i][j] = 0;
		}
	}

	for (int i = 0; i < 20; i++)
	{
		map[10][12+i] = 1;
	}
}

EnemyComponent::~EnemyComponent() { }

void EnemyComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	auto enemy = entity->getComponent<Component::TransformComponent>();
	std::shared_ptr<Hydra::World::IEntity> playerEntity = getPlayerComponent();
	auto player = playerEntity->getComponent<Component::PlayerComponent>();

	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;
	_debugState = _pathState;

	if (_startPosition == glm::vec3(0, 0, 0))
	{
		enemy->setPosition(_position);
		_startPosition = enemy->getPosition();
	}

	switch (_enemyID)
	{
		case EnemyTypes::Alien:
		{
			switch (_pathState)
			{
			case SEARCHING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) < 10.0f)
				{
					_isAtGoal = true;
					_pathState = ATTACKING;
				}

				_pathFinding->findPath(enemy->getPosition(), player->getPosition(), map);
				_isAtGoal = false;
				if (_pathFinding->foundGoal)
				{
					_targetPos = player->getPosition();
					_pathState = FOUND_GOAL;
				}
			}break;
			case FOUND_GOAL:
			{
				if (!_isAtGoal)
				{
					glm::vec3 targetDistance = _pathFinding->nextPathPos(enemy->getPosition(), getRadius()) - enemy->getPosition();

					angle = atan2(targetDistance.x, targetDistance.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));

					glm::vec3 direction = glm::normalize(targetDistance);

					_velocityX = 0.1f * direction.x;
					_velocityZ = 0.1f * direction.z;

					if (glm::length(_targetPos - enemy->getPosition()) < 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(_targetPos - player->getPosition()) > 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(enemy->getPosition() - player->getPosition()) < 7.0f)
					{
						_isAtGoal = true;
						_pathState = ATTACKING;
					}
				}
			}break;
			case ATTACKING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) > 8.0f)
				{
					_pathFinding->intializedStartGoal = false;
					_pathFinding->foundGoal = false;
					_pathFinding->clearPathToGoal();
					_pathState = SEARCHING;
				}
				else
				{
					glm::vec3 playerDir = player->getPosition() - enemy->getPosition();
					angle = atan2(playerDir.x, playerDir.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
				}
			}break;
			}

			_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);
			enemy->setPosition(_position);
			enemy->setRotation(rotation);

		}break;
		case EnemyTypes::Robot:
		{
			switch (_pathState)
			{
			case SEARCHING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) < 10.0f)
				{
					_isAtGoal = true;
					_pathState = ATTACKING;
				}

				_pathFinding->findPath(enemy->getPosition(), player->getPosition(), map);
				_isAtGoal = false;
				if (_pathFinding->foundGoal)
				{
					_targetPos = player->getPosition();
					_pathState = FOUND_GOAL;
				}
			}break;
			case FOUND_GOAL:
			{
				if (!_isAtGoal)
				{
					glm::vec3 targetDistance = _pathFinding->nextPathPos(enemy->getPosition(), getRadius()) - enemy->getPosition();

					angle = atan2(targetDistance.x, targetDistance.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));

					glm::vec3 direction = glm::normalize(targetDistance);

					_velocityX = 0.1f * direction.x;
					_velocityZ = 0.1f * direction.z;

					if (glm::length(_targetPos - enemy->getPosition()) < 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(_targetPos - player->getPosition()) > 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(enemy->getPosition() - player->getPosition()) < 7.0f)
					{
						_isAtGoal = true;
						_pathState = ATTACKING;
					}
				}
			}break;
			case ATTACKING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) > 8.0f)
				{
					_pathFinding->intializedStartGoal = false;
					_pathFinding->foundGoal = false;
					_pathFinding->clearPathToGoal();
					_pathState = SEARCHING;
				}
				else
				{
					glm::vec3 playerDir = player->getPosition() - enemy->getPosition();
					angle = atan2(playerDir.x, playerDir.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
				}
			}break;
			}

			_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);
			enemy->setPosition(_position);
			enemy->setRotation(rotation);

		}break;
		case EnemyTypes::AlienBoss:
		{
			switch (_pathState)
			{
			case SEARCHING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) < 10.0f)
				{
					_isAtGoal = true;
					_pathState = ATTACKING;
				}

				_pathFinding->findPath(enemy->getPosition(), player->getPosition(), map);
				_isAtGoal = false;
				if (_pathFinding->foundGoal)
				{
					_targetPos = player->getPosition();
					_pathState = FOUND_GOAL;
				}
			}break;
			case FOUND_GOAL:
			{
				if (!_isAtGoal)
				{
					glm::vec3 targetDistance = _pathFinding->nextPathPos(enemy->getPosition(), getRadius()) - enemy->getPosition();

					angle = atan2(targetDistance.x, targetDistance.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));

					glm::vec3 direction = glm::normalize(targetDistance);

					_velocityX = 0.1f * direction.x;
					_velocityZ = 0.1f * direction.z;

					if (glm::length(_targetPos - enemy->getPosition()) < 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(_targetPos - player->getPosition()) > 7.0f)
					{
						_pathFinding->intializedStartGoal = false;
						_pathFinding->foundGoal = false;
						_pathFinding->clearPathToGoal();
						_pathState = SEARCHING;
					}

					if (glm::length(enemy->getPosition() - player->getPosition()) < 7.0f)
					{
						_isAtGoal = true;
						_pathState = ATTACKING;
					}
				}
			}break;
			case ATTACKING:
			{
				if (glm::length(enemy->getPosition() - player->getPosition()) > 8.0f)
				{
					_pathFinding->intializedStartGoal = false;
					_pathFinding->foundGoal = false;
					_pathFinding->clearPathToGoal();
					_pathState = SEARCHING;
				}
				else
				{
					glm::vec3 playerDir = player->getPosition() - enemy->getPosition();
					angle = atan2(playerDir.x, playerDir.z);
					rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
				}
			}break;
			}

			_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);
			enemy->setPosition(_position);
			enemy->setRotation(rotation);

		}break;
	}

	 //debug for pathfinding
		//int tempX = enemy->getPosition().x;
		//int tempZ = enemy->getPosition().z;
		//map[tempX][tempZ] = 2;
		//for (int i = 0; i < _pathFinding->_visitedList.size(); i++)
		//{
		//	map[(int)_pathFinding->_visitedList[i]->m_xcoord][(int)_pathFinding->_visitedList[i]->m_zcoord] = 3;
		//}
		//for (int i = 0; i < _pathFinding->_openList.size(); i++)
		//{
		//	map[(int)_pathFinding->_openList[i]->m_xcoord][(int)_pathFinding->_openList[i]->m_zcoord] = 3;
		//}
		//for (int i = 0; i < _pathFinding->_pathToEnd.size(); i++)
		//{
		//	map[(int)_pathFinding->_pathToEnd[i]->x][(int)_pathFinding->_pathToEnd[i]->z] = 3;
		//}
}

glm::vec3 EnemyComponent::getPosition()
{
	auto enemy = entity->getComponent<Component::TransformComponent>();

	return enemy->getPosition();
}

float EnemyComponent::getRadius()
{
	return 1.0f;
}

std::shared_ptr<Hydra::World::IEntity> EnemyComponent::getPlayerComponent()
{
	std::shared_ptr<Hydra::World::IEntity> player;
	auto world = entity->getParent()->getChildren();
	for (size_t i = 0; i < world.size(); i++) {
		if (world[i]->getName() == "Player") {
			player = world[i];
		}
	}
	return player;
}

void EnemyComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "startPosition",{ _startPosition.x, _startPosition.y, _startPosition.z } },
		{ "velocityX", _velocityX },
		{ "velocityY", _velocityY },
		{ "velocityZ", _velocityZ },
		{ "enemyID", (int)_enemyID },
		{ "pathState", (int)_pathState }
	};
	for (size_t i = 0; i < 64; i++)
	{
		for (size_t j = 0; j < 64; j++)
		{
			json["map"][i][j] = map[i][j];
		}
	}
}

void EnemyComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& startpos = json["startPosition"];
	_startPosition = glm::vec3{ startpos[0].get<float>(), startpos[1].get<float>(), startpos[2].get<float>() };

	_velocityX = json["velocityX"].get<float>();
	_velocityY = json["velocityY"].get<float>();
	_velocityZ = json["velocityZ"].get<float>();

	_enemyID = (EnemyTypes)json["enemyID"].get<int>();
	_pathState = (PathState)json["pathState"].get<int>();
	for (size_t i = 0; i < 64; i++)
	{
		for (size_t j = 0; j < 64; j++)
		{
			map[i][j] = json["map"][i][j].get<int>();
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
}

int Hydra::Component::EnemyComponent::getWall(int x, int y)
{
	int result = 0;
	if (map[x][y] == 1)
	{
		result = 1;
	}
	else if (map[x][y] == 2)
	{
		result = 2;
	}
	else if (map[x][y] == 3)
	{
		result = 3;
	}
	return result;
}

