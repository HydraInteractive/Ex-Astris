// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* Bullet stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/component/bulletcomponent.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

BulletComponent::BulletComponent(IEntity* entity) : IComponent(entity) {

}
BulletComponent::BulletComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction, float velocity) : IComponent(entity) {
	_position = position;
	_direction = direction;
	_velocity = velocity;
	_interpolation = 0.0f;
	_deleteTimer = SDL_GetTicks();
	_type = BULLET_HOMING;
}

BulletComponent::~BulletComponent() { }

std::vector<IEntity*> BulletComponent::getEnemies()
{
	auto world = entity;	
	while (world->getName() != "World")
		world = world->getParent();

	std::vector<IEntity*> allEnemies;
	auto children = world->getChildren();
	for (size_t i = 0; i < children.size(); i++) {
		if (!children[i]->getName().find("Enemy")) {
			allEnemies.push_back(children[i].get());
		}
	}
	return allEnemies;
}

IEntity* BulletComponent::getClosestEnemy()
{
	IEntity* closestEnemy = nullptr;
	std::vector<IEntity*> allEnemies = getEnemies();
	
	float distance = FLT_MAX;
	size_t enemyNr = -1;
	for (size_t i = 0; i < allEnemies.size(); i++) {
		auto enemy = allEnemies[i]->getComponent<Component::EnemyComponent>();
		if (glm::distance(enemy->getPosition(), _position) < distance)
		{
			distance = glm::distance(enemy->getPosition(), _position);
			enemyNr = i;
		}
	}
	
	if (enemyNr != -1)
		closestEnemy = allEnemies[enemyNr];

	return closestEnemy;
}

IEntity* BulletComponent::getHomingEnemy() {
	IEntity* closestEnemy = nullptr;
	std::vector<IEntity*> allEnemies = getEnemies();

	size_t enemyNr = -1;
	for (size_t i = 0; i < allEnemies.size(); i++) {
		auto enemy = allEnemies[i]->getComponent<Component::EnemyComponent>();
		
		float distance = FLT_MAX;
		glm::vec3 directionToEnemy = glm::normalize(enemy->getPosition() - _position);

		if (glm::distance(_direction, directionToEnemy) < 0.5f && glm::distance(_position, enemy->getPosition()) < distance)
		{
			distance = glm::distance(enemy->getPosition(), _position);
			enemyNr = i;
		}
	}

	if (enemyNr != -1)
		closestEnemy = allEnemies[enemyNr];

	return closestEnemy;
}


void BulletComponent::tick(TickAction action, float delta) {
	switch (_type)
	{
	case BULLET_NORMAL:{
		_position += _velocity * _direction * delta;
	}
	break;
	case BULLET_MAGNETIC:{
		target = getClosestEnemy();

		if (target != nullptr)
		{
			glm::vec3 enemyPos = target->getComponent<Component::EnemyComponent>()->getPosition();
			glm::vec3 dirToEnemy = glm::normalize(enemyPos - _position);
			float interpolateSpeed = 3.0f;
			
			if (glm::distance(dirToEnemy, _direction) < 0.5f)
				_direction = glm::normalize(interpolateSpeed*delta*dirToEnemy + (1.0f - interpolateSpeed*delta)*_direction);
		}
		_position += _velocity * _direction * delta;
	}
	break;
	case BULLET_HOMING:{
		
		if (target == nullptr)
			target = getHomingEnemy();

		if (target != nullptr){
			glm::vec3 enemyPos = target->getComponent<Component::EnemyComponent>()->getPosition();
			glm::vec3 dirToEnemy = glm::normalize(enemyPos - _position);

			_interpolation += 3.0f * delta;
			if (_interpolation > 1.0f)
				_interpolation = 1.0f;

			_direction = glm::normalize(_interpolation*dirToEnemy + (1.0f - _interpolation*delta)*_direction);
		}

		_position += _velocity * _direction * delta;
	}
	break;
	}

	auto transform = entity->getComponent<Component::TransformComponent>();
	transform->setPosition(_position);
	if (target != nullptr && glm::distance(target->getComponent<Component::TransformComponent>()->getPosition(), _position) < 0.5f)
	{
		entity->markDead();
	}
	if (SDL_GetTicks() > _deleteTimer + 4 * 1000) {
		entity->markDead();
	}
}

void BulletComponent::serialize(nlohmann::json& json) const {
	json = {
		{ "position",{ _position.x, _position.y, _position.z } },
		{ "direction",{ _direction.x, _direction.y, _direction.z } },
		{ "velocity", _velocity }
	};
}

void BulletComponent::deserialize(nlohmann::json& json) {
	auto& pos = json["position"];
	_position = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	auto& dir = json["direction"];
	_direction = glm::vec3{ dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>() };

	_velocity = json["velocity"].get<float>();
}

// Register UI buttons in the debug UI
// Note: This function won't always be called
void BulletComponent::registerUI() {
	ImGui::DragFloat3("Position", glm::value_ptr(_position));
	ImGui::DragFloat3("Direction", glm::value_ptr(_direction));
	ImGui::DragFloat("Velocity", &_velocity);
}
