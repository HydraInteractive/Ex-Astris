#include <barcode/editorstate.hpp>

#include <fstream>
#include <json.hpp>
#include <imgui/imgui.h>
#include <barcode/menustate.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
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

using world = Hydra::World::World;
namespace Barcode {
	EditorState::EditorState() : _engine(Hydra::IEngine::getInstance()) {}

	void EditorState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());
		_textFactory = Hydra::IO::GLTextFactory::create("assets/fonts/font.png");
		auto windowSize = _engine->getView()->getSize();
		_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, windowSize);

		{
			_hitboxBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/hitboxdebug.vert", "", "assets/shaders/hitboxdebug.frag", _engine->getView());
			_hitboxBatch.batch.clearFlags = ClearFlags::none;
		}

		{
			_previewBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/previewWindow.vert", "", "assets/shaders/previewWindow.frag", glm::ivec2(720, 720));
			_previewBatch.output->addTexture(0, Hydra::Renderer::TextureType::u8RGB).finalize();
		}
		_initWorld();

		_importerMenu = new ImporterMenu();
		_exporterMenu = new ExporterMenu();
		_componentMenu = new ComponentMenu();
	}

	EditorState::~EditorState() { }

	void EditorState::onMainMenu() {
		if (ImGui::BeginMenu("Editor")) {
			if (ImGui::MenuItem("Import...")) {
				_showImporter = !_showImporter;
				if (_showImporter)
					_importerMenu->refresh("/assets");
			}
			if (ImGui::MenuItem("Export..."))	{
				_showExporter = !_showExporter;
				if (_showExporter)
					_exporterMenu->refresh("/assets");
			}
			if (ImGui::MenuItem("Pathfinding..."))
			{
				_showPathMapCreator = !_showPathMapCreator;
				if (_showPathMapCreator)
				{
					_cc->useOrtho = true;
				}
			}
			if (ImGui::MenuItem("Add component...")){
				_showComponentMenu = !_showComponentMenu;
				if (_showComponentMenu)
					_componentMenu->refresh();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Clear room")) {
				if (FileTree::getRoomEntity() != nullptr)
					FileTree::getRoomEntity()->dead = true;

				auto room = world::newEntity("Workspace", world::root());
				auto t = room->addComponent<Hydra::Component::TransformComponent>();
				auto r = room->addComponent<Hydra::Component::RoomComponent>();

			}		

			ImGui::EndMenu();
		}
	}

	void EditorState::runFrame(float delta) {
		auto windowSize = _engine->getView()->getSize();
		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		_playerSystem.tick(delta);
		_abilitySystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);
		_animationSystem.tick(delta);

		const glm::vec3 cameraPos = _playerTransform->position;

		if (_showPathMapCreator)
		{
			_cc->getTransformComponent()->position = glm::vec3(0, 40, 0);
			_cc->cameraPitch = 1.571;
			_cc->cameraYaw = 0;
			_cc->movementSpeed = 0;
			_cc->sensitivity = 0;
		}

		static bool enableHitboxDebug = true;
		ImGui::Checkbox("Enable Hitbox Debug", &enableHitboxDebug);
		ImGui::Checkbox("Enable Glow", &MenuState::glowEnabled);
		ImGui::Checkbox("Enable SSAO", &MenuState::ssaoEnabled);
		ImGui::Checkbox("Enable Shadow", &MenuState::shadowEnabled);

		auto viewMatrix = _cc->getViewMatrix();
		glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
		glm::vec3 forwardVector = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] };
		_cameraSystem.setCamInternals(*_cc);
		_cameraSystem.setCamDef(_playerTransform->position, forwardVector, upVector, rightVector, *_cc);

		_dgp->render(cameraPos, *_cc, *_playerTransform);

		if (enableHitboxDebug) {
			for (auto& kv : _hitboxBatch.batch.objects)
				kv.second.clear();

			std::vector<std::shared_ptr<Entity>> entities;
			world::getEntitiesWithComponents<Hydra::Component::RigidBodyComponent, Hydra::Component::DrawObjectComponent>(entities);
			for (auto e : entities) {
				// GOTTA MAKE IT VERSATILE SO IT CAN TAKE CAPSULE AS WELL.
				auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
				auto rgbc = e->getComponent<Hydra::Component::RigidBodyComponent>();
				glm::vec3 newScale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(drawObj->modelMatrix, newScale, rotation, translation, skew, perspective);
				_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(rgbc->getHalfExtentScale() * glm::vec3(2)));
			}

			world::getEntitiesWithComponents<Hydra::Component::GhostObjectComponent, Hydra::Component::DrawObjectComponent>(entities);
			for (auto e : entities) {
				auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
				auto goc = e->getComponent<Hydra::Component::GhostObjectComponent>();
				glm::vec3 newScale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(drawObj->modelMatrix, newScale, rotation, translation, skew, perspective);
				_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(glm::translate(translation) * glm::mat4_cast(goc->quatRotation) * glm::scale(goc->halfExtents * glm::vec3(2)));
			}

			_hitboxBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_hitboxBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_engine->getRenderer()->renderHitboxes(_hitboxBatch.batch);
		}

		if (_showImporter)
			_importerMenu->render(_showImporter, &_previewBatch.batch, delta);
		if (_showExporter)
			_exporterMenu->render(_showExporter);
		if (_showPathMapCreator)
			_pathingMenu.render(_showPathMapCreator, delta, _engine->getView()->getSize().x, _engine->getView()->getSize().y);
		if (_showComponentMenu)
			_componentMenu->render(_showComponentMenu, _physicsSystem);
	}
	void EditorState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_particleSystem, &_abilitySystem, &_aiSystem, &_physicsSystem, &_bulletSystem, &_playerSystem, &_rendererSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}
	void EditorState::_initWorld() {
		_hitboxCube = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getMesh("assets/objects/HitBox.mATTIC");
		{
			auto floor = world::newEntity("Floor", world::root());
			auto t = floor->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(0, -7, 0);
			auto rgbc = floor->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createStaticPlane(glm::vec3(0, 1, 0), 1, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_WALL
				, 0, 0, 0, 0.6f, 0);
			floor->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Floor_v2.mATTIC");
		}

		{
			auto room = world::newEntity("Workspace", world::root());
			auto t = room->addComponent<Hydra::Component::TransformComponent>();
			auto r = room->addComponent<Hydra::Component::RoomComponent>();
		}

		{
			auto compass = world::newEntity("Compass", world::root());
			auto t = compass->addComponent<Hydra::Component::TransformComponent>();

			auto n = world::newEntity("North", compass);
			auto tn = n->addComponent<Hydra::Component::TransformComponent>();
			tn->position = glm::vec3(0,0,17);

			auto e = world::newEntity("East", compass);
			auto te = e->addComponent<Hydra::Component::TransformComponent>();
			te->position = glm::vec3(17, 0, 0);

			auto s = world::newEntity("South", compass);
			auto ts = s->addComponent<Hydra::Component::TransformComponent>();
			ts->position = glm::vec3(0, 0, -17);

			auto w = world::newEntity("West", compass);
			auto tw = w->addComponent<Hydra::Component::TransformComponent>();
			tw->position = glm::vec3(-17, 0, 0);
		}

		{
			auto playerEntity = world::newEntity("Player", world::root());
			auto c = playerEntity->addComponent<Hydra::Component::CameraComponent>();
			c->noClip = true;
			c->mouseControl = false;
			auto t = playerEntity->addComponent<Hydra::Component::TransformComponent>();
			_playerTransform = t.get();
		}

		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
			auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(8.0, 0, 3.5);
		}

		{
			BlueprintLoader::save("world.blueprint", "World Blueprint", world::root().get());
			Hydra::World::World::reset();
			auto bp = BlueprintLoader::load("world.blueprint");
			bp->spawn(world::root());
		}

		{
			_cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());
			for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents()) {
				_engine->log(Hydra::LogLevel::normal, "Enabling bullet for %s", world::getEntity(rb->entityID)->name.c_str());
				_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
			}
		}
	}
}
