#include <hydra/system/particlesystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/cameracomponent.hpp>

using namespace Hydra::System;

ParticleSystem::ParticleSystem() {}
ParticleSystem::~ParticleSystem() {}

void ParticleSystem::tick(float delta) {
	using world = Hydra::World::World;
	using Hydra::Component::ParticleComponent;
	constexpr int innerCount =  ParticleComponent::TextureInnerGrid *  ParticleComponent::TextureInnerGrid;
	constexpr float smallImageSize = 1.0f / (ParticleComponent::TextureInnerGrid * ParticleComponent::TextureOuterGrid);

	//Process ParticleComponent
	world::getEntitiesWithComponents<Hydra::Component::ParticleComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto p = entities[i]->getComponent<Hydra::Component::ParticleComponent>();

		const size_t id = static_cast<size_t>(p->texture);
		const glm::vec2 outerOffset = glm::vec2(id %  ParticleComponent::TextureOuterGrid, id /  ParticleComponent::TextureOuterGrid) / (float)  ParticleComponent::TextureOuterGrid;

		{
			p->accumulator += delta;
			p->spawnParticles();
		}

#pragma omp parallel for
		for (int_openmp_t i = 0; i < (int_openmp_t) Hydra::Component::ParticleComponent::MaxParticleAmount /*p->particles.size()*/; i++) {
			auto& particle = p->particles[i];
			particle.life = std::max(0.0f, particle.life - delta);

			particle.transform.position += particle.velocity * delta + (particle.acceleration * delta * delta) / 2.0f;
			particle.transform.dirty = true;
			particle.velocity += particle.acceleration * delta;

			{
				const float lifeFactor = 1 - (particle.life / particle.startLife);
				const int innerID = innerCount * lifeFactor;
				const int nextInnerID = (innerID + 1);

				particle.texOffset1 = outerOffset + glm::vec2(innerID %  ParticleComponent::TextureInnerGrid, innerID /  ParticleComponent::TextureInnerGrid) * smallImageSize;
				particle.texOffset2 = outerOffset + glm::vec2(nextInnerID %  ParticleComponent::TextureInnerGrid, nextInnerID /  ParticleComponent::TextureInnerGrid) * smallImageSize;
				particle.texCoordInfo = glm::vec2(smallImageSize, fmod(innerCount * lifeFactor, 1));
			}
		}
	}

	entities.clear();
}


void ParticleSystem::registerUI() {}

