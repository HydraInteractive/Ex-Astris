#include <barcode/perkEditor.hpp>
#include <barcode/menustate.hpp>
#include <hydra/component/cameracomponent.hpp>

using world = Hydra::World::World;
namespace Barcode
{
	Barcode::PerkEditorState::PerkEditorState() : _engine(Hydra::IEngine::getInstance()){}

	PerkEditorState::~PerkEditorState()
	{
		delete _blurUtil;
	}

	void Barcode::PerkEditorState::onMainMenu()
	{
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Attributes..."))
			{
				_showPerkAttributeMenu = !_showPerkAttributeMenu;
			}
			ImGui::EndMenu();
		}
	}

	void Barcode::PerkEditorState::runFrame(float delta)
	{
		_attribMenu.updateBullet(bullet);

		std::vector<std::shared_ptr<Entity>> entities;
		world::getEntitiesWithComponents<Hydra::Component::MeshComponent>(entities);

		if (bullet.mesh != oldMeshID)
		{
			
			entities[0]->dead = true;

			auto newBullet = world::newEntity("bullet", world::root());
			auto t = newBullet->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(0, 0, -4);
			t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
			t->scale = glm::vec3(1);
			t->dirty = true;



			oldMeshID = bullet.mesh;
			if (bullet.mesh == 0) {
				newBullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Bullet.mATTIC");
			}
			if (bullet.mesh == 1) {
				newBullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Star.mATTIC");
			}
			if (bullet.mesh == 2) {
				newBullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Trident.mATTIC");
			}
			if (bullet.mesh == 3) {
				newBullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Banana.mATTIC");
			}
			if (bullet.mesh == 4) {
				newBullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Duck.mATTIC");
			}
		}
		auto windwSize = _engine->getView()->getSize();

		_cameraSystem.tick(delta);
		_physicsSystem.tick(delta);
		_playerSystem.tick(delta);
		_rendererSystem.tick(delta);

		const glm::vec3 camPos = _playTrans->position;

		auto viewMatrix = _cc->getViewMatrix();
		glm::vec3 rightVec = { viewMatrix[0][0],viewMatrix[1][0] , viewMatrix[2][0] };
		glm::vec3 upVec = { viewMatrix[0][1] ,viewMatrix[1][1] ,viewMatrix[2][1] };
		glm::vec3 forwardVec = { viewMatrix[0][2] ,viewMatrix[1][2] ,viewMatrix[2][2] };
		_cameraSystem.setCamInternals(*_cc);
		_cameraSystem.setCamDef(_playTrans->position, forwardVec, upVec, rightVec, *_cc);

		for (auto& kv : _bulletRender.batch.objects)
			kv.second.clear();

		//std::vector<std::shared_ptr<Entity>> entities;
		//world::getEntitiesWithComponents<Hydra::Component::MeshComponent>(entities);
		glm::vec3 colour(1.0f);
		bool hasGlow = bullet.glow;
		float glowIntensity = bullet.glowIntensity;

		for (int i = 0; i < 3; i++)
		{
			//colour[i] = PerkAttribMenu().Bullet.bulletColor[i];
			colour[i] = bullet.bulletColor[i];
		}
		for (auto e : entities) 
		{
			auto t = e->getComponent<Hydra::Component::TransformComponent>();
			auto mc = e->getComponent<Hydra::Component::MeshComponent>();
			auto drawObj = e->getComponent<Hydra::Component::DrawObjectComponent>()->drawObject;
			/*auto bc = e->getComponent<Hydra::Component::BulletComponent>();
			if (bc)
				hasGlow = bc->glow;*/

			t->dirty = true;
			t->scale = glm::vec3(bullet.bulletSize);
			for (int i = 0; i < 3; i++)
			{
				//colour[i] = glm::normalize(bullet.bulletColor[i]);
			}
			_bulletRender.batch.objects[drawObj->mesh].push_back(drawObj->modelMatrix);
		}

		entities.clear();
		_bulletRender.pipeline->setValue(0, _cc->getViewMatrix());
		_bulletRender.pipeline->setValue(1, _cc->getProjectionMatrix());
		_bulletRender.pipeline->setValue(5, colour); //THIS IS IMPORTANT LINE REMEMBER THIS LINE PLZ
		_bulletRender.pipeline->setValue(6, hasGlow);
		_bulletRender.pipeline->setValue(7, glowIntensity);
		_engine->getRenderer()->render(_bulletRender.batch);

		_blurUtil->blur((*_bulletRender.output)[1], 10, _bulletRender.output->getSize());
		_glowBatch.pipeline->setValue(1, 0);
		_glowBatch.pipeline->setValue(2, 1);
		(*_bulletRender.output)[0]->bind(0);
		_blurUtil->getOutput()->bind(1);
		_engine->getRenderer()->postProcessing(_glowBatch.batch);
		if (_showPerkAttributeMenu)
		{
			_attribMenu.render(_showPerkAttributeMenu, &_bulletRender.batch, delta);
		}
	}

	void PerkEditorState::load()
	{
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());
		_textFactory = Hydra::IO::GLTextFactory::create("assets/fonts/font.png");
		auto windwSize = _engine->getView()->getSize();
		//_dgp = std::make_unique<DefaultGraphicsPipeline>(_cameraSystem, windwSize);

		{
			_bulletRender = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/perkVert.vert","", "assets/shaders/perkFrag.frag", windwSize);
			_bulletRender.output->addTexture(0, Hydra::Renderer::TextureType::u8RGB) // Colors (Orignal image for glow)
				.addTexture(1, Hydra::Renderer::TextureType::u8RGB)
				.finalize(); // Highlighted glow parts.
			_bulletRender.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			_blurUtil = new Barcode::BlurUtil(_engine->getRenderer(), _bulletRender.output->getSize(), Hydra::Renderer::TextureType::u8RGB);
			_glowBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/glow.vert", "", "assets/shaders/glow.frag", _engine->getView());
			//_glowBatch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
		}

		_initWorld();
		_attribMenu = PerkAttribMenu();
		
	}

	void PerkEditorState::_initSystem()
	{
		const std::vector<Hydra::World::ISystem*> system = { _engine->getDeadSystem(), &_cameraSystem,&_physicsSystem,&_playerSystem, &_rendererSystem};
		_engine->getUIRenderer()->registerSystems(system);
	}

	void Barcode::PerkEditorState::_initWorld()
	{
		
	
		
		auto bullet = world::newEntity("bullet", world::root());
		bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Bullet.mATTIC");
		auto t = bullet->addComponent<Hydra::Component::TransformComponent>();
		t->position = glm::vec3(0, 0, -4);
		t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
		t->scale = glm::vec3(1);
		t->dirty = true;
		{
			auto playerEntity = world::newEntity("Player", world::root());
			auto camera = playerEntity->addComponent<Hydra::Component::CameraComponent>();
			camera->noClip = true;
			camera->mouseControl = false;
			auto transform = playerEntity->addComponent<Hydra::Component::TransformComponent>();
			transform->setPosition(glm::vec3(0));
			//transform = glm::vec3(300, 0, 0);
			_playTrans = transform.get();
		}
		{
			auto lightEntity = world::newEntity("Light", world::root());
			auto light = lightEntity->addComponent<Hydra::Component::LightComponent>();
			auto lTransform = lightEntity->addComponent<Hydra::Component::TransformComponent>();
			lTransform->position = glm::vec3(8.0, 0, 3.5);
		}
		{
			_cc = static_cast<Hydra::Component::CameraComponent*>(Hydra::Component::CameraComponent::componentHandler->getActiveComponents()[0].get());
			for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents())
			{
				_engine->log(Hydra::LogLevel::normal, "Enabling rigid body for %s", world::getEntity(rb->entityID)->name.c_str());
				_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
			}
		}
		
	}
}