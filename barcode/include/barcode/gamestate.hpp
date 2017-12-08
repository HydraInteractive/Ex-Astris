/**
 * Game state
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
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/io/textfactory.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>

#include <hydra/system/camerasystem.hpp>
#include <hydra/system/particlesystem.hpp>
#include <hydra/system/abilitysystem.hpp>
#include <hydra/system/lifesystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/playersystem.hpp>
#include <hydra/system/renderersystem.hpp>
#include <hydra/system/spawnersystem.hpp>
#include <hydra/system/soundfxsystem.hpp>
#include <hydra/system/perksystem.hpp>
#include <hydra/system/animationsystem.hpp>
#include <hydra/system/pickupsystem.hpp>
#include <hydra/system/textsystem.hpp>
#include <hydra/system/lightsystem.hpp>

namespace Barcode {
	class GameState final : public Hydra::IState {
	public:
		static char addr[256];
		static int port;

		GameState();
		~GameState() final;

		void onMainMenu() final;
		void load() final;
		int currentFrame = 0;
		void runFrame(float delta) final;

		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::IO::ITextFactory* getTextFactory() final { return _textFactory.get(); }
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem; }

	private:
		Hydra::IEngine* _engine;
		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;
		std::unique_ptr<Hydra::IO::ITextFactory> _textFactory;

		Hydra::System::CameraSystem _cameraSystem;
		Hydra::System::ParticleSystem _particleSystem;
		Hydra::System::AbilitySystem _abilitySystem;
		Hydra::System::LifeSystem _lifeSystem;
		Hydra::System::AISystem _aiSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::BulletSystem _bulletSystem;
		Hydra::System::PlayerSystem _playerSystem;
		Hydra::System::RendererSystem _rendererSystem;
		Hydra::System::SpawnerSystem _spawnerSystem;
		Hydra::System::SoundFxSystem _soundFxSystem;
		Hydra::System::PerkSystem _perkSystem;
		Hydra::System::AnimationSystem _animationSystem;
		Hydra::System::PickUpSystem _pickUpSystem;
		Hydra::System::TextSystem _textSystem;
		Hydra::System::LightSystem _lightSystem;

		std::unique_ptr<DefaultGraphicsPipeline> _dgp;
		RenderBatch<Hydra::Renderer::Batch> _hitboxBatch;

		std::shared_ptr<Hydra::Renderer::IMesh> _hitboxCube;
		std::shared_ptr<Hydra::Renderer::IMesh> _hitboxCapsule;

		bool _paused = false;
		Hydra::World::EntityID _playerID;
		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::TransformComponent* _playerTransform = nullptr;

		float _prevHP = 1;
		float _hpTimeUp = 0;
		int  _loadingScreenPicture = 0;
		float _loadingScreenTimer = 1;
		bool _didConnect = false;

		void _initSystem();
		void _initWorld();
		static void _onPlayerShoot(Hydra::Component::WeaponComponent& weapon, Hydra::World::Entity* bullet, void* userdata);
		static void _onUpdatePVS(nlohmann::json&& json, void* userdata);
		static void _onWin(void* userdata);
	};
}
