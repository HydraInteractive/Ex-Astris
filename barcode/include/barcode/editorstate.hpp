/**
* Editor state
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
#include <hydra/world/blueprintloader.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <hydra/system/bulletphysicssystem.hpp>

#include <imgui/imgui.h>
//#include <barcode/ImporterMenu.hpp>
//#include <barcode/ExporterMenu.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>

#include <hydra/system/camerasystem.hpp>
#include <hydra/system/lightsystem.hpp>
#include <hydra/system/particlesystem.hpp>
#include <hydra/system/abilitysystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/playersystem.hpp>
#include <hydra/system/renderersystem.hpp>

#include <hydra/io/input.hpp>

#include <fstream>
#include <json.hpp>

namespace Barcode {
	class EditorState final : public Hydra::IState {
	public:
		EditorState();
		~EditorState() final;

		void onMainMenu() final;
		void load() final;
		int currentFrame = 0;
		void runFrame(float delta) final;
;
		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem; }

	private:
		//ImporterMenu* _importerMenu;
		//ExporterMenu* _exporterMenu;
		bool _showImporter = false;
		bool _showExporter = false;

		std::string selectedPath;

		struct RenderBatch final {
			std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
			std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
			std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
			std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

			std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
			Hydra::Renderer::Batch batch;
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

		//Image drawn in the render window
		Hydra::Renderer::UIRenderWindow* _finalImage;

		//Render batches
		RenderBatch _geometryBatch; // First part of deferred rendering
		RenderBatch _animationBatch; // AnimationBatch
		RenderBatch _lightingBatch; // Second part of deferred rendering
		RenderBatch _glowBatch; // Glow batch.
		RenderBatch _viewBatch;
		RenderBatch _postTestBatch;
		RenderBatch _shadowBatch;
		RenderBatch _ssaoBatch;
		ParticleRenderBatch _particleBatch;

		//// ParticleTexture
		std::shared_ptr<Hydra::Renderer::ITexture> _particleAtlases;

		// Extra framebuffers, pipeline and shaders for glow/bloom/blur
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO1;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO2;
		std::vector<float> _fiveGaussianKernel1;
		std::vector<float> _fiveGaussianKernel2;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredOriginal;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG1;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG2;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG3;

		std::shared_ptr<Hydra::Renderer::IPipeline> _glowPipeline;
		std::unique_ptr<Hydra::Renderer::IShader> _glowVertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> _glowFragmentShader;

		std::shared_ptr<Hydra::Renderer::IPipeline> _shadowPipeline;
		std::unique_ptr<Hydra::Renderer::IShader> _shadowVertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> _shadowFragmentShader;
		std::shared_ptr<Hydra::Renderer::ITexture> _shadowMap;

		std::shared_ptr<Hydra::Renderer::ITexture> _ssaoNoise;

		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::LightComponent* _light = nullptr;

		//Input _input;

		Hydra::System::CameraSystem _cameraSystem;
		Hydra::System::LightSystem _lightSystem;
		Hydra::System::ParticleSystem _particleSystem;
		Hydra::System::AbilitySystem _abilitySystem;
		Hydra::System::AISystem _aiSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::BulletSystem _bulletSystem;
		Hydra::System::PlayerSystem _playerSystem;
		Hydra::System::RendererSystem _rendererSystem;

		void _initSystem();
		void _initWorld();

		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int nrOfTimes, glm::vec2 size, const std::vector<float>& kernel);
	};
}