/**
 * Game state
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>

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

#include <hydra/io/input.hpp>

namespace Barcode {
	class GameState final : public Hydra::IState {
	public:
		GameState();
		~GameState() final;

		void onMainMenu() final;
		void load() final;
		int currentFrame = 0;
		void runFrame(float delta) final;

		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem; }

	private:
		struct RenderBatch final {
			std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
			std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
			std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
			std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

			std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
			Hydra::Renderer::Batch batch;
		};

		struct AnimationRenderBatch final {
			std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
			std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
			std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
			std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

			std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
			Hydra::Renderer::AnimationBatch batch;
		};

		struct ParticleRenderBatch final {
			std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
			std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
			std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
			std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

			std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
			Hydra::Renderer::ParticleBatch batch;
		};

		Hydra::IEngine* _engine;
		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;

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

		RenderBatch _geometryBatch; // First part of deferred rendering
		AnimationRenderBatch _animationBatch; // AnimationBatch
		RenderBatch _lightingBatch; // Second part of deferred rendering
		RenderBatch _glowBatch; // Glow batch.
		RenderBatch _viewBatch;
		RenderBatch _postTestBatch;
		RenderBatch _shadowBatch;
		AnimationRenderBatch _shadowAnimationBatch;
		RenderBatch _ssaoBatch;
		RenderBatch _hudBatch;
		RenderBatch _hitboxBatch;

		ParticleRenderBatch _particleBatch;

		std::shared_ptr<Hydra::Renderer::ITexture> _animationData;

		// ParticleTexture
		std::shared_ptr<Hydra::Renderer::ITexture> _particleAtlases;

		// Extra framebuffers, pipeline and shaders for glow/bloom/blur
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO1;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO2;

		std::shared_ptr<Hydra::Renderer::IPipeline> _glowPipeline;
		std::unique_ptr<Hydra::Renderer::IShader> _glowVertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> _glowFragmentShader;

		std::shared_ptr<Hydra::Renderer::IPipeline> _shadowPipeline;
		std::unique_ptr<Hydra::Renderer::IShader> _shadowVertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> _shadowFragmentShader;
		std::shared_ptr<Hydra::Renderer::ITexture> _shadowMap;

		std::shared_ptr<Hydra::Renderer::ITexture> _ssaoNoise;
		std::shared_ptr<Hydra::Renderer::IMesh> _hitboxCube;

		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::TransformComponent* _playerTransform = nullptr;
		Hydra::Component::LightComponent* _dirLight = nullptr;

		Input _input;

		void _initSystem();
		void _initWorld();

		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int nrOfTimes, glm::vec2 size);
	};

	//class DemoWindow : public TBWindow
	//{
	//public:
	//	DemoWindow(TBWidget *root);
	//	bool LoadResourceFile(const char *filename);
	//	void LoadResourceData(const char *data);
	//	void LoadResource(TBNode &node);
	//	virtual bool OnEvent(const TBWidgetEvent &ev);
	//};
}
