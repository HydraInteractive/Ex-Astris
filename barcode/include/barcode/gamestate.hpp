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
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>

namespace Barcode {
	class GameState final : public Hydra::IState {
	public:
		GameState();
		~GameState() final;

		void onMainMenu() final;
		void load() final;

		void runFrame(float delta) final;

		inline Hydra::World::IWorld* getWorld() final { return _world.get(); };
		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }

	private:
		struct RenderBatch final {
			std::unique_ptr<Hydra::Renderer::IShader> vertexShader;
			std::unique_ptr<Hydra::Renderer::IShader> geometryShader;
			std::unique_ptr<Hydra::Renderer::IShader> fragmentShader;
			std::unique_ptr<Hydra::Renderer::IPipeline> pipeline;

			std::shared_ptr<Hydra::Renderer::IFramebuffer> output;
			Hydra::Renderer::Batch batch;
		};

		Hydra::IEngine* _engine;
		std::unique_ptr<Hydra::World::IWorld> _world;
		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;

		Hydra::Renderer::UIRenderWindow* _positionWindow;
		Hydra::Renderer::UIRenderWindow* _diffuseWindow;
		Hydra::Renderer::UIRenderWindow* _normalWindow;
		Hydra::Renderer::UIRenderWindow* _glowWindow;
		Hydra::Renderer::UIRenderWindow* _depthWindow;
		Hydra::Renderer::UIRenderWindow* _postTestWindow;

		RenderBatch _geometryBatch; // First part of deferred rendering
		//RenderBatch _geometryBatch; // AnimationBatch
		RenderBatch _lightingBatch; // Second part of deferred rendering
		RenderBatch _glowBatch; // Glow batch.
		RenderBatch _viewBatch;
		RenderBatch _postTestBatch;
		RenderBatch _particleBatch;

		// ParticleTexture
		std::shared_ptr<Hydra::Renderer::ITexture> _particleAtlases;

		// Extra framebuffers, pipeline and shaders for glow/bloom/blur
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO1;
		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurrExtraFBO2;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredOriginal;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG1;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG2;
		std::shared_ptr<Hydra::Renderer::ITexture> _blurredIMG3;

		std::shared_ptr<Hydra::Renderer::IPipeline> _glowPipeline;
		std::unique_ptr<Hydra::Renderer::IShader> _glowVertexShader;
		std::unique_ptr<Hydra::Renderer::IShader> _glowFragmentShader;

		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::PlayerComponent* player = nullptr;
		Hydra::Component::EnemyComponent* _enemy = nullptr;

		void _initWorld();

		std::shared_ptr<Hydra::Renderer::IFramebuffer> _blurGlowTexture(std::shared_ptr<Hydra::Renderer::ITexture>& texture, int &nrOfTimes, glm::vec2 size);
	};
}
