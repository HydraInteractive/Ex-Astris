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
}

EnemyComponent::~EnemyComponent() { }

void EnemyComponent::tick(TickAction action) {
	// If you only have one TickAction in 'wantTick' you don't need to check the tickaction here.

	_velocityX = 0;
	_velocityY = 0;
	_velocityZ = 0;

	auto enemy = entity->getComponent<Component::TransformComponent>();
	if (_startPosition == glm::vec3(0, 0, 0))
	{
		_startPosition = enemy->getPosition();
	}

	if (_enemyID == EnemyTypes::Alien)
	{
		_position = enemy->getPosition();

		if (_position.z < _startPosition.z - 10)
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
		}

		_position = _position + glm::vec3(_velocityX, _velocityY, _velocityZ);

		enemy->setPosition(_position);
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
	}
}

glm::vec3 Hydra::Component::EnemyComponent::getPosition()
{
	auto enemy = entity->getComponent<Component::TransformComponent>();

	return enemy->getPosition();
}

float Hydra::Component::EnemyComponent::getRadius()
{
	return 1.5f;
}

void EnemyComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "startPosition",{ _startPosition.x, _startPosition.y, _startPosition.z } },
		{ "velocityX", _velocityX },
		{ "velocityY", _velocityY },
		{ "velocityZ", _velocityZ },
		{ "enemyID", (int)_enemyID },
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
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void EnemyComponent::registerUI() {
	ImGui::InputFloat("X", &_position.x);
	ImGui::InputFloat("Y", &_position.y);
	ImGui::InputFloat("Z", &_position.z);
	ImGui::InputFloat("startY", &_startPosition.y);
}