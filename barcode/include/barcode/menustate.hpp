/**
 * Mainmenu state
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
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>
#include <hydra/physics/bulletmanager.hpp>

#include <barcode/editorstate.hpp>
#include <barcode/gamestate.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	class MenuState final : public Hydra::IState {
	public:
		MenuState();
		~MenuState() final;

		void onMainMenu() final;
		void load() final;

		void runFrame(float delta) final;

		inline Hydra::World::IWorld* getWorld() final { return _world.get(); };
		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::Physics::IPhysicsManager* getPhysicsManager() final { return _physicsManager.get(); }

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
		std::unique_ptr<Hydra::Physics::IPhysicsManager> _physicsManager;

		RenderBatch _viewBatch;

		void _initWorld();

		int menu = 0;
	};
	//bool ImageAnimButton(ImTextureID user_texture_id, ImTextureID user_texture_id2, const ImVec2 & size, const ImVec2 & uv0, const ImVec2 & uv1, const ImVec2 & uv2, const ImVec2 & uv3, int frame_padding, const ImVec4 & bg_col, const ImVec4 & tint_col);
}
