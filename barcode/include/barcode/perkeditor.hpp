/**
* Perk Editor state
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
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>
#include <hydra/io/gltextfactory.hpp>

#include <hydra/system/camerasystem.hpp>
#include <hydra/system/particlesystem.hpp>
#include <hydra/system/abilitysystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/playersystem.hpp>
#include <hydra/system/renderersystem.hpp>
#include <hydra/system/animationsystem.hpp>

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
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>

#include <imgui/imgui.h>

#include <barcode/pathingmapmenu.hpp>
#include <barcode/importermenu.hpp>
#include <barcode/exportermenu.hpp>

#include <barcode/perkattribmenu.hpp>


#include <fstream>
#include <json.hpp>

namespace Barcode {
	class PerkEditorState final : public Hydra::IState {
	public:
		PerkEditorState();
		~PerkEditorState() final;

		void onMainMenu() final;
		void runFrame(float delta) final;
		void load() final;

		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get();}
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get();}
		inline Hydra::IO::ITextFactory* getTextFactory() final { return _textFactory.get();}
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem;}

	private:
		Hydra::IEngine* _engine;
		
		PerkAttribMenu _attribMenu;
		PerkAttribMenu::BulletInfo bullet;

		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;
		std::unique_ptr<Hydra::IO::ITextFactory> _textFactory;

		Hydra::System::CameraSystem _cameraSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::PlayerSystem _playerSystem;
		Hydra::System::RendererSystem _rendererSystem;

		std::unique_ptr<DefaultGraphicsPipeline> _dgp;
		RenderBatch<Hydra::Renderer::Batch> _bulletRender;
		RenderBatch<Hydra::Renderer::Batch> _glowBatch;
		BlurUtil* _blurUtil;

		Hydra::Component::TransformComponent* _playTrans = nullptr;
		Hydra::Component::CameraComponent* _cc = nullptr;

		bool _showPerkAttributeMenu;

		void _initWorld();
		void _initSystem();

	};
}
