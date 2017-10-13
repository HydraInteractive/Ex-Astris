#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <memory>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	enum EmitterBehaviour : int {PerSecond = 0, Explosion};
	enum ParticleTexture : int {Fire = 0, Knas, BogdanDeluxe};
	
	struct HYDRA_GRAPHICS_API Particle {
		glm::mat4 m;
		glm::vec3 pos;
		glm::vec3 acceleration;
		glm::vec3 vel;
		glm::vec3 scale;
		glm::vec2 texOffset1;
		glm::vec2 texOffset2;
		glm::vec2 texCoordInfo;
		float life;
		float elapsedTime;
		float grav;
		float distanceToCamera;
		bool dead;
		void spawn(glm::vec3 pos, glm::vec3 vel, glm::vec3 acceleration, float life) {
			this->pos = pos;
			this->vel = vel;
			this->acceleration = acceleration;
			this->life = life;
			this->scale = glm::vec3(1.0f);
			this->elapsedTime = 0.f;
			this->dead = false;
			this->texOffset1 = glm::vec2(0);
			this->texOffset2 = glm::vec2(0);
			this->texCoordInfo = glm::vec2(0);
			this->distanceToCamera = 0;
			// Gonna fix rotation soon...
			this->m = glm::translate(pos) * glm::scale(scale);
		}
		void fixMX(glm::quat& rot) { m = glm::translate(pos) * glm::scale(scale); }
	};

	struct HYDRA_GRAPHICS_API ParticleComponent final : public IComponent<ParticleComponent, ComponentBits::Particle> {
		int pps; // Particles per second.
		float accumulator;
		glm::quat tempRotation;
		glm::vec2 offsetToTexture;
		glm::vec3 emitterPos;
		EmitterBehaviour behaviour;
		std::vector<std::shared_ptr<Particle>> particles;

		/*ParticleComponent(IEntity* entity);
		ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, ParticleTexture texture, int nrOfParticles, glm::vec3 pos);*/
		~ParticleComponent() final;

		inline const std::string type() const final { return "ParticleComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
		/*
	private:
		void _generateParticles();
		void _particlePhysics(float delta);
		void _clearDeadParticles();
		void _emmitParticle();
		void _updateTextureCoordInfo(std::shared_ptr<Particle>& p, float delta);
		void _sortParticles(); // Insertion Sort*/
	};
	template struct HYDRA_GRAPHICS_API Hydra::World::IComponent<Hydra::Component::ParticleComponent, Hydra::Component::ComponentBits::Particle>;
};
