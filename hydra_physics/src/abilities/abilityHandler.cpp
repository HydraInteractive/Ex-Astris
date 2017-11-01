#include <hydra/abilities/abilityHandler.hpp>

using world = Hydra::World::World;

AbilityHandler::AbilityHandler(){
	_activeAbility = 0;
	//_abilityList.push_back(&AbilityHandler::mineAbility);
	//_cooldownList.push_back(0);
}
void AbilityHandler::addAbility(void(AbilityHandler::*newAbility)(Hydra::World::Entity*, glm::vec3, glm::vec3))
{
	_abilityList.push_back(newAbility);
	_cooldownList.push_back(0);
}
void AbilityHandler::useAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	if (_cooldownList.size() > 0 && _cooldownList[_activeAbility] == 0)
	{
		(this->*_abilityList[_activeAbility])(abilitiesEntity, position, direction);
		
		//_cooldownList[_activeAbility] = 5;
		
		if (++_activeAbility >= _abilityList.size())
		{
			_activeAbility = 0;
		}
	}
}
void AbilityHandler::grenadeAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	auto grenade = world::newEntity("Grenade", abilitiesEntity);
	auto g = grenade->addComponent<Hydra::Component::GrenadeComponent>();
	g->direction = direction;
	grenade->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Fridge.ATTIC");
	auto t = grenade->addComponent<Hydra::Component::TransformComponent>();
	t->position = position;
}
void AbilityHandler::mineAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	auto mine = world::newEntity("Mine", abilitiesEntity);
	auto m = mine->addComponent<Hydra::Component::MineComponent>();
	m->direction = direction;
	mine->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Fridge.ATTIC");
	auto t = mine->addComponent<Hydra::Component::TransformComponent>();
	t->position = position;
}
