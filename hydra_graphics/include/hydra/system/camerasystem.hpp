#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API CameraSystem final : public Hydra::World::ISystem {
	public:
		enum class FrustrumCheck {
			outside = 0,
			intersect,
			inside
		};

		CameraSystem();
		~CameraSystem() final;

		void tick(float delta) final;

		void setCamInternals(Hydra::Component::CameraComponent& cc);
		void setCamDef(const glm::vec3& cPos, const glm::vec3& cDir, const glm::vec3& up, const glm::vec3& right, Hydra::Component::CameraComponent& cc);
		FrustrumCheck sphereInFrustum(const glm::vec3& p, float radius, Hydra::Component::CameraComponent& cc);

		inline const std::string type() const final { return "CameraSystem"; }
		void registerUI() final;
	};
}
