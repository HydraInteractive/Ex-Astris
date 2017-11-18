#include <barcode/winstate.hpp>

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
	WinState::WinState() : _engine(Hydra::IEngine::getInstance()) {}

	void WinState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, _engine->getView()->getSize());

		_initSystem();
		_initWorld();
	}

	WinState::~WinState() { }

	void WinState::onMainMenu() {}

	void WinState::runFrame(float delta) {
		_physicsSystem.tick(delta);
		_cameraSystem.tick(delta);
		_particleSystem.tick(delta);
		_rendererSystem.tick(delta);
		_animationSystem.tick(delta);

		ImGui::Checkbox("Enable Glow", &MenuState::glowEnabled);
		ImGui::Checkbox("Enable SSAO", &MenuState::ssaoEnabled);
		ImGui::Checkbox("Enable Shadow", &MenuState::shadowEnabled);

		_robotMesh->currentFrame = 0;
		_alienMesh->currentFrame = 0;

		//(*_lightRotation) = glm::normalize((*_lightRotation) *glm::angleAxis(10 * delta, glm::vec3{0.2, 0.5, 0.8}));

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

		ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.2f), ImGuiCond_Always, ImVec2(0.5, 0.5));
		ImGui::Begin("Title", nullptr, windowFlags);
		{
			_engine->getUIRenderer()->pushFont(UIFont::big);
			ImGui::Text("You won!");
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

		ImGui::PopStyleColor();
	}

	void WinState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = { _engine->getDeadSystem(), &_cameraSystem, &_particleSystem, &_physicsSystem, &_rendererSystem, &_animationSystem };
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void WinState::_initWorld() {
		using world = Hydra::World::World;

		{
			auto p = world::newEntity("Player", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{-0.175, -0.036, -0.991};
			t->rotation = glm::quat{0.985, 0, -0.175, 0};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/PlayerModel.mATTIC");
			m->animationIndex = 2;
		}

		{
			auto p = world::newEntity("Robot", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{1.024, -0.402, -0.977};
			t->rotation = glm::quat{-0.834, 0.522, 0.127, 0.129};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/RobotModel.mATTIC");
			m->animationIndex = 0;
			m->currentFrame = 0;
			_robotMesh = m.get(); {
				auto particleEmitter = world::newEntity("Energy particles", p);
				particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("QUAD");
				auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
				p->delay = 1.0f / 32.0f;
				p->texture = Hydra::Component::ParticleComponent::ParticleTexture::Energy;
				p->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::Explosion;
				auto t = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3{ 0.2, 2.28, 0.64 };
				t->rotation = glm::quat{ 0.273, 0.303, -0.019, 0.913};
			}
		}

		{
			auto p = world::newEntity("Alien", world::root());
			auto t = p->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{3.393, 1.117, 1.471};
			t->rotation = glm::quat{0.629, -0.73, -0.171, -0.206};
			auto m = p->addComponent<Hydra::Component::MeshComponent>();
			m->loadMesh("assets/objects/characters/AlienModel.mATTIC");
			m->animationIndex = 0;
			m->currentFrame = 0;
			_alienMesh = m.get(); {
				auto particleEmitter = world::newEntity("Blood", p);
				particleEmitter->addComponent<Hydra::Component::MeshComponent>()->loadMesh("QUAD");
				auto p = particleEmitter->addComponent<Hydra::Component::ParticleComponent>();
				p->delay = 1.0f / 32.0f;
				p->texture = Hydra::Component::ParticleComponent::ParticleTexture::AlienBlood;
				p->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::PerSecond;
				auto t = particleEmitter->addComponent<Hydra::Component::TransformComponent>();
				t->position = glm::vec3{ 0.43, 0.93, -0.16 };
				t->rotation = glm::quat{ 0.710, 0.605, 0.265, -0.244};
			}
		}

		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto l = lightEntity->addComponent<Hydra::Component::LightComponent>();
			l->color = glm::vec3{1, 1, 1};
			auto t = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 3, 2, 2 };
			t->rotation = glm::quat{ -0.496, 0.811, -0.117, -0.288 };
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
