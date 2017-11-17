/**
 * Mainmenu state
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#include <barcode/renderingutils.hpp>

#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/camerasystem.hpp>
#include <hydra/system/particlesystem.hpp>
#include <hydra/system/renderersystem.hpp>
#include <hydra/system/animationsystem.hpp>

namespace Barcode {
	class LoseState final : public Hydra::IState {
	public:
		LoseState();
		~LoseState() final;

		void onMainMenu() final;
		void load() final;

		void runFrame(float delta) final;

		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem; }

	private:
		Hydra::IEngine* _engine;
		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;

		Hydra::System::CameraSystem _cameraSystem;
		Hydra::System::ParticleSystem _particleSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::RendererSystem _rendererSystem;
		Hydra::System::AnimationSystem _animationSystem;

		std::unique_ptr<DefaultGraphicsPipeline> _dgp;
		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::TransformComponent* _cameraTransform = nullptr;
		glm::quat* _lightRotation;

		void _initSystem();
		void _initWorld();
	};
}
