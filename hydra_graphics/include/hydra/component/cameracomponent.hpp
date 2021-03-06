/**
 * A camera component that generates view and projection matrices.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <hydra/component/transformcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {

	struct HYDRA_GRAPHICS_API CameraComponent final : public IComponent<CameraComponent, ComponentBits::Camera> {
		bool useOrtho = false;
		float orthoLeft = -17.0f;
		float orthoRight = 17.0f;
		float orthoBottom = -17.0f;
		float orthoTop = 17.0f;
		
		float fov = 90.0f;
		float zNear = 0.1f;
		float zFar = 250.0f;
		float aspect = 1920.0f/1080.0f;

		float sensitivity = 0.003f;
		float cameraYaw = 0.0f;
		float cameraPitch = 0.0f;
		bool mouseControl = true;

		bool noClip = false;
		float movementSpeed = 10; // Only for noClip
		float shiftMultiplier = 5; // Only for noClip

		enum {
			TOP = 0, BOTTOM, LEFT, RIGHT, NEARP, FARP
		};

		struct Plane {
			glm::vec3 p0, p1, p2, n = glm::vec3(0,0,0);
			void set3Points(glm::vec3 pZero, glm::vec3 pOne, glm::vec3 pTwo) { p0 = pZero; p1 = pOne; p2 = pTwo; }
			float distance(glm::vec3 p) {
				glm::vec3 edge0 = p1 - p0;
				glm::vec3 edge1 = p2 - p0;
				n = glm::cross(edge0, edge1);
				n = glm::normalize(n);

				float D = glm::dot(-n, p0);
				return glm::dot(n, p) + D;
			}
		};

		Plane pl[6];

		glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr = glm::vec3();
		float nearD, farD, tang = 0.0f;
		float nw, nh, fw, fh = 0.0f;

		~CameraComponent() final;

		inline const std::string type() const final { return "CameraComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() { return glm::translate(glm::mat4_cast(getTransformComponent()->rotation), -getTransformComponent()->position); }
		//inline glm::mat4 getViewMatrix() { return glm::lookAt(getTransformComponent()->position, getTransformComponent()->position + getTransformComponent()->getDirection(), glm::vec3(0, 1, 0)); }
		inline glm::mat4 getProjectionMatrix() const 
		{ 
			if (useOrtho)
			{
				return glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, zNear, zFar);
			}
			else
			{
				return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
			}
		}
		
		inline std::shared_ptr<Hydra::Component::TransformComponent> getTransformComponent() { return Hydra::World::World::getEntity(entityID)->getComponent<Hydra::Component::TransformComponent>();}
	};
};
