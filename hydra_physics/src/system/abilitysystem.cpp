#include <hydra/system/abilitysystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <algorithm>

#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/particlecomponent.hpp>



using namespace Hydra::System;

AbilitySystem::AbilitySystem() {}
AbilitySystem::~AbilitySystem() {}

void AbilitySystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process GrenadeComponent
	world::getEntitiesWithComponents<Hydra::Component::GrenadeComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto g = entities[i]->getComponent<Hydra::Component::GrenadeComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		auto s = entities[i]->getComponent<Hydra::Component::SoundFxComponent>();
		g->detonateTimer -= delta;

		if (g->detonateTimer <= 0 && !g->isExploding) {
			s->soundsToPlay.push_back("assets/sounds/piano.wav");
			g->isExploding = true;

			std::vector<std::shared_ptr<Hydra::World::Entity>> allEnemies;
			world::getEntitiesWithComponents<Hydra::Component::AIComponent>(allEnemies);
			for (int_openmp_t i = 0; i < (int_openmp_t)allEnemies.size(); i++) {
				auto tc = allEnemies[i]->getComponent<Hydra::Component::TransformComponent>();
				auto lc = allEnemies[i]->getComponent<Hydra::Component::LifeComponent>();
				
				if (glm::length(tc->position - t->position) <= 20.0f)
				{
					lc->applyDamage(15);
				}
			}
			_spawnParticleEmitterAt(t->position, glm::vec3(0,1,0));
			allEnemies.clear();
			entities[i]->dead = true;
		}
	}

	//Process MineComponent
	world::getEntitiesWithComponents<Hydra::Component::MineComponent, Hydra::Component::TransformComponent>(entities);
	#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto m = entities[i]->getComponent<Hydra::Component::MineComponent>();
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
	}

	entities.clear();
}

void AbilitySystem::registerUI() {}

void AbilitySystem::_spawnParticleEmitterAt(const glm::vec3 & pos, const glm::vec3 & normal)
{
	auto pE = Hydra::World::World::newEntity("Collision Particle Spawner", Hydra::World::World::rootID);

	pE->addComponent<Hydra::Component::MeshComponent>()->loadMesh("PARTICLEQUAD");

	auto pETC = pE->addComponent<Hydra::Component::TransformComponent>();
	pETC->position = pos;
	pETC->scale = glm::vec3(4.0f, 4.0f, 4.0f);

	auto pEPC = pE->addComponent<Hydra::Component::ParticleComponent>();
	pEPC->delay = 1.0f / 1.0f;
	pEPC->accumulator = 40.0f;
	pEPC->tempVelocity = glm::vec3(40.0f, 40.0f, 40.0f);
	pEPC->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::Explosion;
	pEPC->texture = Hydra::Component::ParticleComponent::ParticleTexture::Energy;
	pEPC->optionalNormal = normal;

	auto pELC = pE->addComponent<Hydra::Component::LifeComponent>();
	pELC->maxHP = 0.6f;
	pELC->health = 0.6f;
}
