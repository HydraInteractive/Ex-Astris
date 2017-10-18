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
	struct HYDRA_GRAPHICS_API ParticleComponent final : public IComponent<ParticleComponent, ComponentBits::Particle> {
		enum class EmitterBehaviour : int { PerSecond = 0, Explosion, MAX_COUNT };
		static constexpr const char* EmitterBehaviourStr[] = { "PerSecond", "Explosion" };
		enum class ParticleTexture : int { Fire = 0, Knas, BogdanDeluxe, MAX_COUNT };
		static constexpr const char* ParticleTextureStr[] = { "Fire", "Knas", "BogdanDeluxe" };

		struct HYDRA_GRAPHICS_API Particle {
			glm::vec3 position;
			glm::vec3 acceleration;
			glm::vec3 velocity;
			glm::vec3 scale = glm::vec3(1.0f);
			glm::vec2 texOffset1;
			glm::vec2 texOffset2;
			glm::vec2 texCoordInfo;
			float life;
			float startLife;
			float grav;
			float distanceToCamera;
			void spawn(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, float life) {
				this->position = position;
				this->velocity = velocity;
				this->acceleration = acceleration;
				this->life = life;
			}
			glm::mat4 getMatrix() { return glm::translate(position) * glm::scale(scale); }
		};

		static constexpr size_t TextureOuterGrid = 6; // 6x6
		static constexpr size_t TextureInnerGrid = 4; // 4x4
		static constexpr float ParticleSize = 1.0f / (TextureInnerGrid * TextureOuterGrid);

		float delay = 1; // 0.1 = 10 Particle/Second
		float accumulator = 0;
		glm::vec3 emitterPos = glm::vec3{0, 0, 0};
		EmitterBehaviour behaviour = EmitterBehaviour::PerSecond;
		ParticleTexture texture = ParticleTexture::Fire;
		std::vector<std::shared_ptr<Particle>> particles;

		~ParticleComponent() final;

		void spawnParticles();

		inline const std::string type() const final { return "ParticleComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
