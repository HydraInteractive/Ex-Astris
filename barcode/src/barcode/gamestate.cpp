#include <barcode/gamestate.hpp>

#include <barcode/menustate.hpp>
#include <barcode/losestate.hpp>
#include <barcode/winstate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <glm/gtx/matrix_decompose.hpp>

#include <hydra/world/blueprintloader.hpp>
#include <imgui/imgui.h>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>

using world = Hydra::World::World;

namespace Barcode {
	GameState::GameState() : _engine(Hydra::IEngine::getInstance()) {}

	void GameState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());
		_textFactory = Hydra::IO::GLTextFactory::create("assets/fonts/font.png");

		auto windowSize = _engine->getView()->getSize();
		_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, windowSize);

		{
			_hitboxBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/hitboxdebug.vert", "", "assets/shaders/hitboxdebug.frag", _engine->getView());
			_hitboxBatch.batch.clearFlags = ClearFlags::none;
		}

		_initWorld();
	}

	GameState::~GameState() { }

	void GameState::onMainMenu() { }

	void GameState::runFrame(float delta) {

		auto windowSize = _engine->getView()->getSize();

		if (!world::getEntity(_playerID)) {
			_engine->setState<LoseState>();
			return;
		}

		{
			static std::vector<std::shared_ptr<Entity>> _enemies;
			world::getEntitiesWithComponents<Hydra::Component::AIComponent, Hydra::Component::LifeComponent>(_enemies);
			if (!_enemies.size()) {
				_enemies.clear();
				_engine->setState<WinState>();
				return;
			}
			_enemies.clear();
		}

		bool oldPaused = _paused;
		if (ImGui::IsKeyPressed(SDLK_ESCAPE, false)) {
			_paused = !_paused;

			if (_paused)
				ImGui::OpenPopup("Pause Menu");
		}

		if (ImGui::BeginPopupModal("Pause Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("The game is paused");

			ImGui::Separator();

			if (ImGui::Button("Continue"))
				_paused = false;
			ImGui::SameLine();
			if (ImGui::Button("Return to the mainmenu")) {
				_engine->setState<Barcode::MenuState>();
				ImGui::EndPopup();
				return;
			}
			if (!_paused)
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (_paused != oldPaused) {
			static bool oldMouseControl;
			static bool oldNoClip;

			if (_paused) {
				oldMouseControl = _cc->mouseControl;
				oldNoClip = _cc->noClip;
				_cc->mouseControl = false;
				_cc->noClip = false;
			} else {
				_cc->mouseControl = oldMouseControl;
				_cc->noClip = oldNoClip;
			}
		}

		if (_paused)
			delta = 0;

		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		_playerSystem.tick(delta);
		_abilitySystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);
		_animationSystem.tick(delta);
		_spawnerSystem.tick(delta);
		_soundFxSystem.tick(delta);
		_perkSystem.tick(delta);
		_lifeSystem.tick(delta);
		_pickUpSystem.tick(delta);
		_textSystem.tick(delta);


		static bool enableHitboxDebug = false;
/*		ImGui::Checkbox("Enable Hitbox Debug", &enableHitboxDebug);
		ImGui::Checkbox("Enable Glow", &MenuState::glowEnabled);
		ImGui::Checkbox("Enable SSAO", &MenuState::ssaoEnabled);
		ImGui::Checkbox("Enable Shadow", &MenuState::shadowEnabled);*/

		const glm::vec3& cameraPos = _playerTransform->position;
		auto viewMatrix = _cc->getViewMatrix();
		glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
		glm::vec3 forwardVector = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] }; //glm::cross(rightVector, upVector);
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
				_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(glm::translate(rgbc->getPosition()) * glm::mat4_cast(rgbc->getRotation()) * glm::scale(rgbc->getHalfExtentScale() * glm::vec3(2)));
			}

			world::getEntitiesWithComponents<Hydra::Component::GhostObjectComponent, Hydra::Component::DrawObjectComponent>(entities);
			for (auto e : entities) {
				auto goc = e->getComponent<Hydra::Component::GhostObjectComponent>();
				_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(goc->getMatrix()*glm::scale(glm::vec3(2)));
				//_hitboxBatch.batch.objects[_hitboxCube.get()].push_back(glm::translate(translation) * glm::mat4_cast(goc->quatRotation) * glm::scale(goc->halfExtents * glm::vec3(2)));
			}
			_hitboxBatch.pipeline->setValue(0, _cc->getViewMatrix());
			_hitboxBatch.pipeline->setValue(1, _cc->getProjectionMatrix());
			_engine->getRenderer()->renderHitboxes(_hitboxBatch.batch);
		}

		{ // Hud windows
		  //static float f = 0.0f;
		  //static bool b = false;
		  //static float invisF[3] = { 0, 0, 0 };
			float hpP = 100;
			float ammoP = 100;
			float degrees = 0;
			//std::vector<Buffs> perksList;
			for (auto& p : Hydra::Component::PlayerComponent::componentHandler->getActiveComponents()) {
				auto player = static_cast<Hydra::Component::PlayerComponent*>(p.get());
				auto weaponc = ((Hydra::Component::WeaponComponent*)player->getWeapon()->getComponent<Hydra::Component::WeaponComponent>().get());
				auto lifeC = ((Hydra::Component::LifeComponent*)Hydra::World::World::getEntity(player->entityID)->getComponent<Hydra::Component::LifeComponent>().get());
				hpP = 100 * ((float)lifeC->health / (float)lifeC->maxHP);
				ammoP = 100 * ((float)weaponc->currmagammo / (float)weaponc->maxmagammo);
				//perksList = player->activeBuffs.getActiveBuffs();
			}
			for (auto& camera : Hydra::Component::CameraComponent::componentHandler->getActiveComponents())
				degrees = glm::degrees(static_cast<Hydra::Component::CameraComponent*>(camera.get())->cameraYaw);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, float(0.0f));

			const int x = _engine->getView()->getSize().x / 2;
			const ImVec2 pos = ImVec2(x, _engine->getView()->getSize().y / 2);

			if (prevHP > hpP) {
				//prevHP = (1 - (delta*3)) * prevHP + (delta*3) * hpP; //LERP
				if (hpTimeUp == 0) {
					hpTimeUp = 1;
				}

				hpTimeUp -= delta;

				if (hpTimeUp >= 0) {
					ImGui::SetNextWindowPos(ImVec2(0, 0));
					ImGui::SetNextWindowSize(ImVec2(1920, 1080));
					ImGui::Begin("DamageBleed", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/blood.png")->getID()), ImVec2(1920, 1080), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1,1,1, hpTimeUp));
					ImGui::End();
				}
				else {
					prevHP = hpP;
					hpTimeUp = 0;
				}
			}
			//Crosshair
			ImGui::SetNextWindowPos(ImVec2(-10 + pos.x, 1 + pos.y));
			ImGui::SetNextWindowSize(ImVec2(20, 20));
			ImGui::Begin("Crosshair", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Crosshair.png")->getID()), ImVec2(20, 20));
			ImGui::End();

			//AimRing
			ImGui::SetNextWindowPos(ImVec2(-51 + pos.x, -42 + pos.y));
			ImGui::SetNextWindowSize(ImVec2(120, 120));
			ImGui::Begin("AimRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AimRing.png")->getID()), ImVec2(100, 100));
			ImGui::End();

			//Hp bar on ring
			float offsetHpF = 72 * hpP * 0.01;
			int offsetHp = offsetHpF;
			ImGui::SetNextWindowPos(ImVec2(-47 + pos.x, -26 + 72 - offsetHp + pos.y));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("HpOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/HpOnRing.png")->getID()), ImVec2(22, offsetHp), ImVec2(0, 1 - hpP * 0.01), ImVec2(1, 1));
			ImGui::End();

			//Ammo on bar
			float offsetAmmoF = 72 * ammoP * 0.01;
			int offsetAmmo = offsetAmmoF;
			ImGui::SetNextWindowPos(ImVec2(+25 + pos.x, -26 + 72 - offsetAmmo + pos.y));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("AmmoOnRing", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/AmmoOnRing.png")->getID()), ImVec2(22, offsetAmmo), ImVec2(0, 1 - ammoP * 0.01), ImVec2(1, 1));
			ImGui::End();

			//Compass that turns with player
			float degreesP = ((float(100) / float(360) * degrees) / 100);
			float degreesO = float(1000) * degreesP;
			ImGui::SetNextWindowPos(ImVec2(pos.x - 275, +70));
			ImGui::SetNextWindowSize(ImVec2(600, 20));
			ImGui::Begin("Compass", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/CompassCut.png")->getID()), ImVec2(550, 20), ImVec2(degreesO / float(1000), 0), ImVec2((float(1) - ((float(450) - degreesO) / float(1000))), 1));
			_textureLoader->getTexture("assets/hud/CompassCut.png")->setRepeat();
			ImGui::End();

			//Enemies on compass
			int i = 0;
			glm::mat4 viewMat = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get())->getViewMatrix();
			std::vector<std::shared_ptr<Entity>> aiEntities;
			world::getEntitiesWithComponents<Hydra::Component::AIComponent>(aiEntities);
			for (auto& enemy : aiEntities) {
				char buf[128];
				snprintf(buf, sizeof(buf), "AI is a scrub here is it's scrubID: %d", i);
				auto playerP = _playerTransform->position;
				auto enemyP = enemy->getComponent<Hydra::Component::TransformComponent>()->position;
				auto enemyDir = glm::normalize(enemyP - playerP);

				glm::vec3 forward(-viewMat[0][2], -viewMat[1][2], -viewMat[2][2]);
				glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

				glm::vec2 forward2D = glm::normalize(glm::vec2(forward.x, forward.z));
				glm::vec2 right2D = glm::normalize(glm::vec2(right.x, right.z));
				glm::vec2 enemy2D = glm::normalize(glm::vec2(enemyDir.x, enemyDir.z));

				float dotPlacment = glm::dot(forward2D, enemy2D); // -1 - +1
				float leftRight = glm::dot(right2D, enemy2D);
				if (leftRight < 0)
				{
					leftRight = 1;
				}
				else
				{
					leftRight = -1;
				}
				if (dotPlacment < 0)
				{
					dotPlacment = 0;
				}
				dotPlacment = dotPlacment;
				dotPlacment = leftRight * (1 - dotPlacment) * 275;
				ImGui::SetNextWindowPos(ImVec2(x + dotPlacment, 75)); //- 275
				ImGui::SetNextWindowSize(ImVec2(20, 20));
				ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
				ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Red.png")->getID()), ImVec2(10, 10));
				ImGui::End();
				i++;
			}

			//Dynamic cooldown dots
			size_t amountOfActives = 3;
			int coolDownList[64] = { 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
			float pForEatchDot = float(1) / float(amountOfActives);
			float stepSize = float(70) * pForEatchDot;
			for (size_t i = 0; i < amountOfActives; i++)
			{
				char buf[128];
				snprintf(buf, sizeof(buf), "Cooldown%lu", i);
				float yOffset = float(stepSize * float(i + 1));
				float xOffset = pow(abs((yOffset - (stepSize / float(2)) - float(35))) * 0.1069, 2);
				ImGui::SetNextWindowPos(ImVec2(-64 + xOffset + pos.x, -24 + yOffset - ((stepSize + 10.0) / 2.0) + pos.y));
				ImGui::SetNextWindowSize(ImVec2(15, 15));
				ImGui::Begin(buf, NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
				if (coolDownList[i] >= 7)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Red.png")->getID()), ImVec2(10, 10));
				}
				else if (coolDownList[i] <= 0)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Green.png")->getID()), ImVec2(10, 10));
				}
				else
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/hud/Yellow.png")->getID()), ImVec2(10, 10));
				}

				ImGui::End();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
		}
	}

	void GameState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_particleSystem, &_abilitySystem, &_aiSystem, &_physicsSystem, &_bulletSystem, &_playerSystem, &_rendererSystem, &_spawnerSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void GameState::_initWorld() {
		_hitboxCube = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getMesh("assets/objects/HitBox.mATTIC");
		{
			auto floor = world::newEntity("Floor", world::root());
			auto t = floor->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(0, 0, 0);
			auto rgbc = floor->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createStaticPlane(glm::vec3(0, 1, 0), 1, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_WALL
				, 0, 0, 0, 0.6f, 0);
			//floor->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Floor_v2.mATTIC");

		}

		{
			auto pickUpEntity = world::newEntity("PickUp", world::root());
			auto t = pickUpEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(5, 5, 5);
			pickUpEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Lock.mATTIC");
			pickUpEntity->addComponent<Hydra::Component::PickUpComponent>();
			auto rgbc = pickUpEntity->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(2.0f, 1.5f, 1.7f), glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PICKUP_OBJECT, 10);
			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
			
			auto pickupText = world::newEntity("Textpickup", world::root());
			pickupText->addComponent<Hydra::Component::MeshComponent>()->loadMesh("TEXTQUAD");
			pickupText->addComponent<Hydra::Component::TransformComponent>()->setPosition(t->position);
			auto textStuff = pickupText->addComponent<Hydra::Component::TextComponent>();
			textStuff->setText("\x01Perk picked up\x02");
			textStuff->isStatic = true;
		}
		{
			//Remove this to gain frames like never before
			tileGen = new TileGeneration("assets/room/tryTwo.room");
			pathfindingMap = tileGen->buildMap();
		}

		/*for (size_t i = 0; i < 1; i++) {
			auto pickUpEntity = world::newEntity("PickUp", world::root());
			auto t = pickUpEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(-5, -5.0f, -4.0f);
			pickUpEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/GreenCargoBox.mATTIC");
			pickUpEntity->addComponent<Hydra::Component::PickUpComponent>();
			auto goc = pickUpEntity->addComponent<Hydra::Component::GhostObjectComponent>();
			goc->createBox(glm::vec3(3,3,3),Hydra::System::BulletPhysicsSystem::COLL_WALL);
		}*/

		{
			auto playerEntity = world::newEntity("Player", world::root());
			auto p = playerEntity->addComponent<Hydra::Component::PlayerComponent>();
			auto c = playerEntity->addComponent<Hydra::Component::CameraComponent>();
			auto h = playerEntity->addComponent<Hydra::Component::LifeComponent>();
			auto m = playerEntity->addComponent<Hydra::Component::MovementComponent>();
			auto s = playerEntity->addComponent<Hydra::Component::SoundFxComponent>();
			auto perks = playerEntity->addComponent<Hydra::Component::PerkComponent>();
			h->health = h->maxHP = 100.0f * MenuState::playerHPMultiplier;
			prevHP = h->health;
			m->movementSpeed = 250.0f;
			auto t = playerEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(-10, 0, -10);
			_playerTransform = t.get();
			auto rgbc = playerEntity->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(1.0f, 2.0f, 1.0f) * t->scale, glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER, 100,
				0, 0, 0.0f, 0);
			rgbc->setAngularForce(glm::vec3(0, 0, 0));

			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
			{
				auto weaponEntity = world::newEntity("Weapon", playerEntity);
				weaponEntity->addComponent<Hydra::Component::WeaponComponent>();
				weaponEntity->getComponent<Hydra::Component::WeaponComponent>()->bulletSize /= 2;
				weaponEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Gun.mATTIC");
				auto t2 = weaponEntity->addComponent<Hydra::Component::TransformComponent>();
				t2->position = glm::vec3(2, -7, -2);
				t2->scale = glm::vec3(0.1f, 0.1f, 0.1f);
				t2->rotation = glm::quat(0, 0, 1, 0);
				t2->ignoreParent = true;
			}
		}

		/*{
			auto alienEntity = world::newEntity("Alien1", world::root());
			alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
			auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
			a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
			a->behaviour->setPathMap(pathfindingMap);
			a->damage = 4;
			a->behaviour->originalRange = 5;
			a->radius = 1;

			auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
			h->maxHP = 80;
			h->health = 80;
			auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
			m->movementSpeed = 8.0f;
			auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ -10, 0, -10 };
			t->scale = glm::vec3{ 1,1,1 };
			t->rotation = glm::vec3{ 0, 90, 0 };
			auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(0.5f, 1.5f, 0.5f) * t->scale, glm::vec3(0, 3, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
				0, 0, 0.6f, 1.0f);
			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
			rgbc->setAngularForce(glm::vec3(0));
		}

		{
			auto robotEntity = world::newEntity("Robot1", world::root());
			robotEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/RobotModel.mATTIC");
			auto a = robotEntity->addComponent<Hydra::Component::AIComponent>();
			a->behaviour = std::make_shared<RobotBehaviour>(robotEntity);
			a->behaviour->setPathMap(pathfindingMap);
			a->damage = 7;
			a->behaviour->originalRange = 20;
			a->radius = 1;
			auto w = robotEntity->addComponent<Hydra::Component::WeaponComponent>();
			w->bulletSpread = 0.3f;
			w->fireRateRPM = 50;
			w->bulletsPerShot = 1;
			w->damage = 5;
			w->maxmagammo = 100000000;
			w->currmagammo = 100000000;
			w->maxammo = 100000000;

			auto h = robotEntity->addComponent<Hydra::Component::LifeComponent>();
			h->maxHP = 70;
			h->health = 70;
			auto m = robotEntity->addComponent<Hydra::Component::MovementComponent>();
			m->movementSpeed = 5.0f;
			auto t = robotEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ -10, 0, -10 };
			t->scale = glm::vec3{ 1,1,1 };
			t->rotation = glm::vec3{ 0, 90, 0 };
			auto rgbc = robotEntity->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(0.5f) * t->scale, glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
				0, 0, 0.6f, 1.0f);
			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		}*/

		//{
		//	auto textEntity = world::newEntity("Bogdan", world::root());
		//	textEntity->addComponent<Hydra::Component::TransformComponent>()->setScale(glm::vec3(10));
		//	textEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("TEXTQUAD");

		//	auto textC = textEntity->addComponent<Hydra::Component::TextComponent>();
		//	textC->setText("Bogdan Here");
		//}

		//{
		//	auto pointLight1 = world::newEntity("Pointlight1", world::root());
		//	pointLight1->addComponent<Hydra::Component::TransformComponent>();
		//	//pointLight1->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/EscapePodDoor.mATTIC");
		//	auto p1LC = pointLight1->addComponent<Hydra::Component::PointLightComponent>();
		//	p1LC->color = glm::vec3(1, 1, 1);
		//} {
		//	auto pointLight2 = world::newEntity("Pointlight2", world::root());
		//	auto t = pointLight2->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3(45, 0, 0);
		//	//pointLight2->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
		//	auto p2LC = pointLight2->addComponent<Hydra::Component::PointLightComponent>();
		//	p2LC->color = glm::vec3(1, 1, 1);
		//} {
		//	auto pointLight3 = world::newEntity("Pointlight3", world::root());
		//	auto t = pointLight3->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3(45, 0, 0);
		//	//pointLight3->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
		//	auto p3LC = pointLight3->addComponent<Hydra::Component::PointLightComponent>();
		//	p3LC->color = glm::vec3(1, 1, 1);
		//}
		//{
		//	auto pointLight4 = world::newEntity("Pointlight4", world::root());
		//	auto t = pointLight4->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3(45, 0, 0);
		//	//pointLight4->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/CylinderContainer.mATTIC");
		//	auto p4LC = pointLight4->addComponent<Hydra::Component::PointLightComponent>();
		//	p4LC->color = glm::vec3(1, 1, 1);
		//}

		{
			auto parent = world::newEntity("Parent", world::root());
			auto tp = parent->addComponent<Hydra::Component::TransformComponent>();
			tp->position = glm::vec3{ 0, 0, 10 };
			//parent->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BigMonitor.mATTIC");

			{
				auto child = world::newEntity("child", parent);
				auto t = child->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3{ 1, 0, 0 };
				//child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");
				{
					auto parent = world::newEntity("Parent", world::root());
					auto tp = parent->addComponent<Hydra::Component::TransformComponent>();
					tp->position = glm::vec3{ 0, 0, 10 };
					//parent->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BigMonitor.mATTIC");

					{
						auto child = world::newEntity("child", parent);
						auto t = child->addComponent<Hydra::Component::TransformComponent>();
						t->position = glm::vec3{ 1, 0, 0 };
						//child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");
					}
					{
						auto child = world::newEntity("child", parent);
						auto t = child->addComponent<Hydra::Component::TransformComponent>();
						t->position = glm::vec3{ -1, 0, 0 };
						//child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");
					}
				}

				{
					auto particleEmitter = world::newEntity("ParticleEmitter", world::root());
					particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("PARTICLEQUAD");
					auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
					p->delay = 1.0f / 1.0f;
					auto t1 = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
					t1->position = glm::vec3{ 4, 0, 4 };

					auto child = world::newEntity("child", parent);
					auto t2 = child->addComponent<Hydra::Component::TransformComponent>();
					t2->position = glm::vec3{ -1, 0, 0 };
					//child->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SourceCode_Monitor.mATTIC");

					auto lightEntity = world::newEntity("Light", world::root());
					auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
					auto t3 = lightEntity->addComponent<Hydra::Component::TransformComponent>();
					t3->position = glm::vec3(8.0, 0, 3.5);
				}

				{
					_cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());
					_playerID = _cc->entityID;

					for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents()) {
						_engine->log(Hydra::LogLevel::normal, "Enabling bullet for %s", world::getEntity(rb->entityID)->name.c_str());
						_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
					}
					for (auto& goc : Hydra::Component::GhostObjectComponent::componentHandler->getActiveComponents()) {
						auto tc = Hydra::World::World::getEntity(goc->entityID)->getComponent<TransformComponent>();
						auto ghostobject = Hydra::World::World::getEntity(goc->entityID)->getComponent<GhostObjectComponent>();
						glm::vec3 newScale;
						glm::quat rotation;
						glm::vec3 translation;
						glm::vec3 skew;
						glm::vec4 perspective;
						glm::decompose(tc->getMatrix(), newScale, rotation, translation, skew, perspective);
						
						ghostobject->ghostObject->setWorldTransform(btTransform(btQuaternion(ghostobject->quatRotation.x, ghostobject->quatRotation.y, ghostobject->quatRotation.z, ghostobject->quatRotation.w), btVector3(translation.x, translation.y, translation.z)));

						_physicsSystem.enable(static_cast<Hydra::Component::GhostObjectComponent*>(goc.get()));
					}
				}
			}
		}
	}

}
