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
	
	HYDRA_API struct Particle {
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
			this->grav = 3.14159265357989f;
			this->texOffset1 = glm::vec2(0);
			this->texOffset2 = glm::vec2(0);
			this->texCoordInfo = glm::vec2(0);
			this->distanceToCamera = 0;
			// Gonna fix rotation soon...
			this->m = glm::translate(pos) * glm::scale(scale);
		}
		void fixMX(glm::quat& rot) { m = glm::translate(pos) * glm::scale(scale); }
	};

	class HYDRA_API ParticleComponent final : public IComponent{
	public:
		ParticleComponent(IEntity* entity);
		ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, int nrOfParticles, glm::vec3 pos);
		~ParticleComponent() final;

		void tick(TickAction action, float delta) final;
		inline TickAction wantTick() const final { return TickAction::physics | TickAction::renderTransparent; }

		inline const std::string type() const final { return "ParticleComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
		const std::vector<std::shared_ptr<Particle>>& getParticles() { return _particles; }
		const Hydra::Renderer::DrawObject* getDrawObject() { return _drawObject; }

	private:
		int _pps; // Particles per second.
		float _accumulator;
		glm::quat _tempRotation;
		glm::vec3 _emitterPos;
		EmitterBehaviour _behaviour;
		Hydra::Renderer::DrawObject* _drawObject;
		std::vector<std::shared_ptr<Particle>> _particles;
		void _generateParticles();
		void _particlePhysics(float delta);
		void _clearDeadParticles();
		void _emmitParticle();
		void _updateTextureCoordInfo(std::shared_ptr<Particle>& p, float delta);
		void _sortParticles(int left, int right); // Quick sort
		void _setTextureOffset(glm::vec2& offset, int index);
	};
};