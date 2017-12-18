#include <barcode/losestate.hpp>

#include <barcode/gamestate.hpp>
#include <barcode/menustate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <imgui/imgui.h>

#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/lightcomponent.hpp>

namespace Barcode {
	LoseState::LoseState() : _engine(Hydra::IEngine::getInstance()) {}

	void LoseState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());
		_textFactory = Hydra::IO::GLTextFactory::create("assets/fonts/font.png");

		_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, _engine->getView()->getSize());
		_dgp->disablePVS = true;

		_initSystem();
		_initWorld();
	}

	LoseState::~LoseState() { }

	void LoseState::onMainMenu() {}

	void LoseState::runFrame(float delta) {
		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);
		_animationSystem.tick(delta);

		(*_lightRotation) = glm::normalize((*_lightRotation) *glm::angleAxis(10 * delta, glm::vec3{0.2, 0.5, 0.8}));

		auto viewMatrix = _cc->getViewMatrix();
		glm::vec3 rightVector = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		glm::vec3 upVector = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
		glm::vec3 forwardVector = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] }; //glm::cross(rightVector, upVector);
		_cameraSystem.setCamInternals(*_cc);
		_cameraSystem.setCamDef(_cameraTransform->position, forwardVector, upVector, rightVector, *_cc);

		_dgp->render(_cameraTransform->position, *_cc, *_cameraTransform);

		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

		auto s = _engine->getView()->getSize();

		ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.2f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::Begin("Title", nullptr, windowFlags);
		{
			_engine->getUIRenderer()->pushFont(UIFont::big);
			ImGui::Text("You died!");
			_engine->getUIRenderer()->popFont();
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.75f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		float scale = 1.0 - (128.0 / _engine->getView()->getSize().x);
		ImGui::Begin("Buttons", nullptr, windowFlags);
		ImGui::SetWindowFontScale(scale);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 32);
			ImGui::Dummy(ImVec2(0, 64 * scale));
			if (ImGui::Button("Main Menu", ImVec2((128 * 2 + 32) * scale, (64 + 32) * scale)))
				_engine->setState<MenuState>();

			if (ImGui::Button("Reconnect", ImVec2((128 * 2 + 32) * scale, (64 + 32) * scale)))
				_engine->setState<GameState>();

			ImGui::Dummy(ImVec2(0, 32 * scale));
			if (ImGui::Button("Quit", ImVec2((128 * 2 + 32) * scale, (64 + 32) * scale)))
				_engine->quit();
			ImGui::PopStyleVar();
		}
		ImGui::End();

		ImGui::PopStyleColor();
	}

	void LoseState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_particleSystem, &_physicsSystem, &_rendererSystem, &_animationSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void LoseState::_initWorld() {
		using world = Hydra::World::World;

		{
			auto p = world::newEntity("Dead player", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->rotation = glm::quat{0.05f, -0.777f, -0.645f, 0.005f};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/PlayerModel2.mATTIC");
			m->animationIndex = 2;

			static const glm::vec3 positions[] = {
				{ 0.06f, 1.81f, 0.22f },
				{ 0.06f, 2.34f, -0.05f },
				{ 0.21f, 3.31f, -0.2f },
				{ 0.32f, 3.95f, 0.39f }
			};
			for (const glm::vec3& pos :  positions) {
				auto particleEmitter = world::newEntity("Blood", p);
				particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("PARTICLEQUAD");
				auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
				p->delay = 1.0f / 128.0f;
				p->texture = Hydra::Component::ParticleComponent::ParticleTexture::Blood;
				p->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::Explosion;
				p->tempVelocity = glm::vec3(6.0f, 6.0f, 6.0f);
				auto t = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
				t->position = pos;
				//t->rotation = glm::quat{ 0, 0, -1, 0 };
				t->rotation = glm::quat{ -0.483f, 0.160f, -0.861f, -0.018f };
			}
		}

		{
			auto p = world::newEntity("Robot", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{5.048f, -0.611f, -1.495f};
			t->rotation = glm::quat{0.939f, 0.0f, -0.343f, 0.0f};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/RobotModel2.mATTIC");
			m->animationIndex = 1;
		}

		{
			auto p = world::newEntity("Alien", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{1.237f, -0.076f, -0.871f};
			t->rotation = glm::quat{-0.908f, -0.318f, -0.243f, 0.122f};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/AlienModel2.mATTIC");
			m->animationIndex = 2;
		}

		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
			l->color = glm::vec3{1.f, 0.f, 0.f};
			auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 3.f, 2.f, 2.f };
			t->rotation = glm::quat{ 0.943f, -0.102f, 0.265f, 0.173f };
			_lightRotation = &t->rotation;
		}
		{
			auto camera = world::newEntity("Camera", world::root());
			_cc = camera->addComponent<Hydra::Component::CameraComponent>().get();
			_cc->noClip = false;
			_cc->mouseControl = false;
			_cc->cameraYaw = -0.15f;
			_cc->cameraPitch = 0.3f; // 0.051;
			auto t = camera->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 4.f, 3.f, 6.f };
			_cameraTransform = t.get();
		}
	}
}
