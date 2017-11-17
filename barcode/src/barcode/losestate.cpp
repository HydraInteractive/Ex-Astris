#include <barcode/losestate.hpp>

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

		_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, _engine->getView()->getSize());

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
		//ImGui::PushFont(engine.getBigFont());
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		/*ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.1, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05, 0.05, 0.05, 1));*/

		auto s = _engine->getView()->getSize();

		ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.2f), ImGuiCond_Always, ImVec2(0.5, 0.5));
		//float scale = 1.0 - (128.0 / _engine->getView()->getSize().x);
		ImGui::Begin("Title", nullptr, windowFlags);
		//ImGui::SetWindowFontScale(scale);
		{
			_engine->getUIRenderer()->pushFont(UIFont::big);
			ImGui::Text("You died!");
			_engine->getUIRenderer()->popFont();
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.75f), ImGuiCond_Always, ImVec2(0.5, 0.5));
		float scale = 1.0 - (128.0 / _engine->getView()->getSize().x);
		ImGui::Begin("Buttons", nullptr, windowFlags);
		ImGui::SetWindowFontScale(scale);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 32);
			ImGui::Dummy(ImVec2(0, 64 * scale));
			if (ImGui::Button("Main Menu", ImVec2((128 * 2 + 32) * scale, (64 + 32) * scale)))
				_engine->setState<MenuState>();

			ImGui::Dummy(ImVec2(0, 32 * scale));
			if (ImGui::Button("Quit", ImVec2((128 * 2 + 32) * scale, (64 + 32) * scale)))
				_engine->quit();
			ImGui::PopStyleVar();
		}
		ImGui::End();

		//ImGui::PopStyleColor(3);
		ImGui::PopStyleColor();
		//ImGui::PopFont();
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
			t->rotation = glm::quat{0.05, -0.777, -0.645, 0.005};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/PlayerModel.mATTIC");
			m->animationIndex = 2;

			static const glm::vec3 positions[] = {
				{ 0.06, 1.81, 0.22 },
				{ 0.06, 2.34, -0.05 },
				{ 0.21, 3.31, -0.2 },
				{ 0.32, 3.95, 0.39 }
			};
			for (const glm::vec3& pos :  positions) {
				auto particleEmitter = world::newEntity("Blood", p);
				particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("QUAD");
				auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
				p->delay = 1.0f / 128.0f;
				p->texture = Hydra::Component::ParticleComponent::ParticleTexture::Blood;
				p->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::Explosion;
				auto t = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
				t->position = pos;;
				//t->rotation = glm::quat{ 0, 0, -1, 0 };
				t->rotation = glm::quat{ -0.483, 0.160, -0.861, -0.018 };
			}
		}

		{
			auto p = world::newEntity("Robot", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{5.048, -0.611, -1.495};
			t->rotation = glm::quat{0.939, 0, -0.343, 0};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/RobotModel.mATTIC");
			m->animationIndex = 1;
		}

		{
			auto p = world::newEntity("Alien", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{1.237, -0.076, -0.871};
			t->rotation = glm::quat{-0.908, -0.318, -0.243, 0.122};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/AlienModel.mATTIC");
			m->animationIndex = 2;
		}

		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
			l->color = glm::vec3{1, 0, 0};
			auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 3, 2, 2 };
			t->rotation = glm::quat{ 0.943, -0.102, 0.265, 0.173 };
			_lightRotation = &t->rotation;
		}
		{
			auto camera = world::newEntity("Camera", world::root());
			_cc = camera->addComponent<Hydra::Component::CameraComponent>().get();
			_cc->noClip = false;
			_cc->mouseControl = false;
			_cc->cameraYaw = -0.15;
			_cc->cameraPitch = 0.3; // 0.051;
			auto t = camera->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 4, 3, 6 };
			_cameraTransform = t.get();
		}
	}
}
