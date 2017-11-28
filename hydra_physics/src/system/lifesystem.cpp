#include <hydra/system/lifesystem.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

#include <hydra/ext/openmp.hpp>

using namespace Hydra::System;

LifeSystem::LifeSystem() {
	
}

LifeSystem::~LifeSystem() {
	
}

void LifeSystem::tick(float delta) {
	using world = Hydra::World::World;

	world::getEntitiesWithComponents<Hydra::Component::LifeComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto lifeC = entities[i]->getComponent<Hydra::Component::LifeComponent>();
		if (lifeC->health <= 0) {
			if (entities[i]->hasComponent<Hydra::Component::AIComponent>()) {
				auto a = entities[i]->getComponent<Hydra::Component::AIComponent>();
				auto m = entities[i]->getComponent<Hydra::Component::MeshComponent>();
				if (a->behaviour->type == Behaviour::Type::ALIEN) {
					if (m->animationIndex != 3) {
						m->currentFrame = 2;	//I'd rather have this at 1 if possible
						m->animationIndex = 3;
					}
					if (m->currentFrame <= 1)	//And this at >= 26
						entities[i]->dead = true;
				}
				else if (a->behaviour->type == Behaviour::Type::ROBOT) {
					if (m->animationIndex != 3) {
						m->currentFrame = 1;
						m->animationIndex = 3;
					}
					if (m->currentFrame >= 26)
						entities[i]->dead = true;
				}
				else if (a->behaviour->type == Behaviour::Type::ALIENBOSS) {
					entities[i]->dead = true;
				}
			}
			else
				entities[i]->dead = true;

			if (auto enemy = entities[i]->getComponent<AIComponent>()){
				auto activeAbilities = enemy->getPlayerEntity()->getComponent<PerkComponent>()->activeAbilities;
				
				for (size_t i = 0; i < activeAbilities.size(); i++){
					activeAbilities[i]->cooldown--;
				}
			}
		}

		if (entities[i]->getComponent<Hydra::Component::ParticleComponent>() || entities[i]->getComponent<Hydra::Component::TextComponent>())
			lifeC->health -= 1 * delta;
	}

	entities.clear();
}

void LifeSystem::registerUI() {
	
}
