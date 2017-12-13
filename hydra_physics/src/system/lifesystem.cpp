#include <hydra/system/lifesystem.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/perkcomponent.hpp>

#include <hydra/ext/openmp.hpp>

using namespace Hydra::System;

LifeSystem::LifeSystem() {
	
}

LifeSystem::~LifeSystem() {
	
}

void LifeSystem::tick(float delta) {
	using world = Hydra::World::World;
	_isKilled.clear();

	world::getEntitiesWithComponents<Hydra::Component::LifeComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto lifeC = entities[i]->getComponent<Hydra::Component::LifeComponent>();
		if (lifeC->health <= 0) {
			entities[i]->dead = true;
			_isKilled.push_back(lifeC->entityID);
			//if (auto enemy = entities[i]->getComponent<AIComponent>()){
			//	auto activeAbilities = enemy->getPlayerEntity()->getComponent<PerkComponent>()->activeAbilities;
			//	
			//	for (size_t i = 0; i < activeAbilities.size(); i++){
			//		activeAbilities[i]->cooldown--;
			//	}
			//}
		}

		if (lifeC->tickDownWithTime || entities[i]->getComponent<Hydra::Component::ParticleComponent>() || entities[i]->getComponent<Hydra::Component::TextComponent>())
			lifeC->health -= 1 * delta;
	}

	entities.clear();
}

void LifeSystem::registerUI() {
	
}
