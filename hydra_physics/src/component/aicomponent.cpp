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

EnemyComponent::EnemyComponent(IEntity* entity) : IComponent(entity), _enemyID(EnemyTypes::Alien) {}

EnemyComponent::EnemyComponent(IEntity* entity, EnemyTypes enemyID) : IComponent(entity), _enemyID(enemyID) {
	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;

	_startPosition = glm::vec3(0, 0, 0);
	_patrolPointReached = false;
	_falling = false;
	_pathState = SEARCHING;
	lastTime = 0;
}

EnemyComponent::~EnemyComponent() { }

void EnemyComponent::tick(TickAction action, float delta) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;
	_debugState = _pathState;

	auto enemy = entity->getComponent<Component::TransformComponent>();
	std::shared_ptr<Hydra::World::IEntity> playerEntity = getPlayerComponent();
	auto player = playerEntity->getComponent<Component::PlayerComponent>();
	if (_startPosition == glm::vec3(0, 0, 0))
	{
		_startPosition = enemy->getPosition();
	}

	if (_enemyID == EnemyTypes::Alien)
	{
		_position = enemy->getPosition();

		/*switch (_pathState)
		{
			case SEARCHING:
			{
				_pathFinding->findPath(enemy->getPosition(), glm::vec3(1.0f, 0.0f, 1.0f));
				if (_pathFinding->foundGoal)
				{
					_pathState = FOUND_GOAL;
				}
			}
			case FOUND_GOAL:
			{
				glm::vec3 targetDistance = _pathFinding->nextPathPos(enemy->getPosition(), getRadius()) - enemy->getPosition();
				float angle = glm::degrees(atan2(targetDistance.x, targetDistance.z));
				enemy->setRotation(glm::angleAxis(angle, glm::vec3(0, 1, 0)));
				_position.x -= 0.1f * sinf(angle * 3.14159 / 180);
				_position.z += 0.1f * cosf(angle * 3.14159 / 180);
			}
		}*/

		if (_pathState == SEARCHING)
		{
			if (glm::length(enemy->getPosition() - player->getPosition()) < 10.0f)
			{
				_isAtGoal = true;
				_pathState = ATTACKING;
			}

			_pathFinding->findPath(enemy->getPosition(), player->getPosition());
			_isAtGoal = false;
			if (_pathFinding->foundGoal)
			{
				_targetPos = player->getPosition();
				_pathState = FOUND_GOAL;
			}
		}
		if (_pathState == FOUND_GOAL)
		{
			if (!_isAtGoal)
			{
				glm::vec3 targetDistance = _pathFinding->nextPathPos(enemy->getPosition(), getRadius()) - enemy->getPosition();
				angle = glm::degrees(atan2(targetDistance.x, targetDistance.z));
				rotation = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0));
				_velocityX -= 0.1f * cos(angle);
				_velocityZ += 0.1f * sin(angle);

				if (glm::length(_targetPos - enemy->getPosition()) < 10.0f)
				{
					_pathFinding->intializedStartGoal = false;
					_pathFinding->foundGoal = false;
					_pathFinding->clearPathToGoal();
					_pathState = SEARCHING;
				}

				if (glm::length(_targetPos - player->getPosition()) > 10.0f)
				{
					_pathFinding->intializedStartGoal = false;
					_pathFinding->foundGoal = false;
					_pathFinding->clearPathToGoal();
					_pathState = SEARCHING;
				}

				if (glm::length(enemy->getPosition() - player->getPosition()) < 10.0f)
				{
					_isAtGoal = true;
					_pathState = ATTACKING;
				}
			}
		}
		if (_pathState == ATTACKING)
		{
			if (glm::length(enemy->getPosition() - player->getPosition()) > 10.0f)
			{
				_pathFinding->intializedStartGoal = false;
				_pathFinding->foundGoal = false;
				_pathFinding->clearPathToGoal();
				_pathState = SEARCHING;
			}
			else
			{
				glm::vec3 playerDir = enemy->getPosition() - player->getPosition();
				angle = glm::degrees(atan2(playerDir.x, playerDir.z));
				rotation = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0));
			}
		}

		/*if (_position.z < _startPosition.z - 10)
		{
			_patrolPointReached = true;

		}
		else if (_position.z > _startPosition.z + 10)
		{
			_patrolPointReached = false;
		}

		if (_patrolPointReached == false)
		{
			_velocityZ -= 0.1f;
		}
		else if (_patrolPointReached == true)
		{
			_velocityZ += 0.1f;
		}*/

		_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);
		enemy->setPosition(_position);
		//glm::quat rotation = glm::angleAxis(atan2(-_velocityX, -_velocityZ), glm::vec3(0, 1, 0)) * glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0));
		enemy->setRotation(rotation);
		
	}
	else if (_enemyID == EnemyTypes::Robot)
	{
		_position = enemy->getPosition();

		if (_position.x < _startPosition.x - 10)
		{
			_patrolPointReached = true;

		}
		else if (_position.x > _startPosition.x + 10)
		{
			_patrolPointReached = false;
		}

		if (_patrolPointReached == false)
		{
			_velocityX -= 0.1f;
		}
		else if (_patrolPointReached == true)
		{
			_velocityX += 0.1f;
		}

		_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);

		enemy->setPosition(_position);
		glm::quat rotation = glm::angleAxis(atan2(-_velocityX, -_velocityZ), glm::vec3(0, 1, 0)) * glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0));
		enemy->setRotation(rotation);
	}
	else if (_enemyID == EnemyTypes::AlienBoss)
	{
		enemy->setScale(glm::vec3(2.5f, 2.5f, 2.5f));

		_position = enemy->getPosition();

		if (glm::length(_position.y - _startPosition.y) < 8.0f && _falling == false)
		{
			_velocityY = -0.9f;
		}

		if (glm::length(_position.y - _startPosition.y) > 7.0f)
		{
			_falling = true;
		}

		if (_falling)
		{
			_velocityY += 0.4f;

			if (fabs(_position.y - _startPosition.y) < 0.5f)
			{
				_velocityY = 0;
				_falling = false;
			}
		}

		_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);

		enemy->setPosition(_position);
		glm::quat rotation = glm::angleAxis(atan2(-_velocityX, -_velocityZ), glm::vec3(0, 1, 0)) * glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0));
		enemy->setRotation(rotation);
	}
	
	
}

glm::vec3 EnemyComponent::getPosition()
{
	auto enemy = entity->getComponent<Component::TransformComponent>();

	return enemy->getPosition();
}

float EnemyComponent::getRadius()
{
	return 5.0f;
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
		{ "pathState", (int)_pathState },
	};
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

bool Hydra::Component::EnemyComponent::getWall(int x, int y)
{
	bool result = _pathFinding->returnWall(x, y);

	return result;
}

