#include <hydra/abilities/abilityHandler.hpp>

AbilityHandler::AbilityHandler(){
	_activeAbility = 0;
	_abilityList.push_back(&AbilityHandler::mineAbility);
	_cooldownList.push_back(0);
}
void AbilityHandler::useAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	if (_cooldownList[_activeAbility] == 0)
	{
		(this->*_abilityList[_activeAbility])(abilitiesEntity, position, direction);
		
		//_cooldownList[_activeAbility] = 5;
		
		if (++_activeAbility >= _abilityList.size())
		{
			_activeAbility = 0;
		}
	}
}
void AbilityHandler::grenadeAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	auto grenade = abilitiesEntity->createEntity("Grenade");
	grenade->addComponent<Hydra::Component::GrenadeComponent>(direction);
	grenade->addComponent<Component::MeshComponent>("assets/objects/Fridge.ATTIC");
	grenade->addComponent<Component::TransformComponent>(position);
}
void AbilityHandler::mineAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction) {
	auto mine = abilitiesEntity->createEntity("Mine");
	mine->addComponent<Hydra::Component::MineComponent>(direction);
	mine->addComponent<Component::MeshComponent>("assets/objects/Fridge.ATTIC");
	mine->addComponent<Component::TransformComponent>(position);
}
