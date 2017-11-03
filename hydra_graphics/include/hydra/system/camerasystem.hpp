#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API CameraSystem final : public Hydra::World::ISystem {
	public:
		CameraSystem();
		~CameraSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "CameraSystem"; }
		void registerUI() final;
		void setCamInternals(Hydra::Component::CameraComponent& cc);
		void setCamDef(glm::vec3& cPos, glm::vec3& cDir, glm::vec3& up, glm::vec3& right, Hydra::Component::CameraComponent& cc);
		//int pointInFrustum(glm::vec3& p, Hydra::Component::CameraComponent& cc);
		int sphereInFrustum(glm::vec3& p, float radius, Hydra::Component::CameraComponent& cc);
	};
}
