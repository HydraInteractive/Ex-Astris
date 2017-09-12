#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	enum EmitterBehaviour : int {PerSecond = 0, Explosion};

	HYDRA_API struct Particle {
		glm::vec3 pos;
		glm::vec3 vel;
		glm::mat4 m;
		float life;
		float elapsedTime;
		void fixMX() {
			
		}


	};

	class HYDRA_API ParticleComponent final : public IComponent{
	public:
		ParticleComponent(IEntity* entity);
		ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, int nrOfParticles);
		~ParticleComponent() final;

		void tick(TickAction action) final;
		inline TickAction wantTick() const final { return TickAction::render; }

		inline const std::string type() const final { return "ParticleComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		int _nrOfParticles;
		EmitterBehaviour _behaviour;
		Hydra::Renderer::DrawObject* _drawObject;
		std::vector<std::shared_ptr<Particle>> _particles;

	};
};
