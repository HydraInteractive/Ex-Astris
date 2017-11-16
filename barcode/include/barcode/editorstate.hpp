/**
* Editor state
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
#include <hydra/world/blueprintloader.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

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

#include <imgui/imgui.h>
#include <barcode/importermenu.hpp>
#include <barcode/exportermenu.hpp>
#include <barcode/filetree.hpp>
#include <barcode/componentmenu.hpp>

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

		inline Hydra::IO::ITextureLoader* getTextureLoader() final { return _textureLoader.get(); }
		inline Hydra::IO::IMeshLoader* getMeshLoader() final { return _meshLoader.get(); }
		inline Hydra::World::ISystem* getPhysicsSystem() final { return &_physicsSystem; }

	private:
		Hydra::IEngine* _engine;
		std::unique_ptr<Hydra::IO::ITextureLoader> _textureLoader;
		std::unique_ptr<Hydra::IO::IMeshLoader> _meshLoader;

		Hydra::System::CameraSystem _cameraSystem;
		Hydra::System::ParticleSystem _particleSystem;
		Hydra::System::AbilitySystem _abilitySystem;
		Hydra::System::AISystem _aiSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::BulletSystem _bulletSystem;
		Hydra::System::PlayerSystem _playerSystem;
		Hydra::System::RendererSystem _rendererSystem;
		Hydra::System::AnimationSystem _animationSystem;

		std::unique_ptr<DefaultGraphicsPipeline> _dgp;
		RenderBatch<Hydra::Renderer::Batch> _hitboxBatch;
		std::shared_ptr<Hydra::Renderer::IMesh> _hitboxCube;
		RenderBatch<Hydra::Renderer::Batch> _previewBatch;

		Hydra::Component::CameraComponent* _cc = nullptr;
		Hydra::Component::TransformComponent* _playerTransform = nullptr;

		ComponentMenu _componentMenu;
		FileTree* _importerMenu;
		FileTree* _exporterMenu;

		bool _showComponentMenu = false;
		bool _showImporter = false;
		bool _showExporter = false;

		std::string _selectedPath;

		void _initSystem();
		void _initWorld();
	};
}
