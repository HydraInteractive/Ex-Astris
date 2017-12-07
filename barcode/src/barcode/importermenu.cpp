#include <barcode/importermenu.hpp>
#include <hydra/engine.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

ImporterMenu::ImporterMenu() : FileTree()
{
	this->workingDir = _getExecutableDir();
	this->_root = nullptr;
	this->_extWhitelist = { ".mATTIC", ".mattic", ".room", ".ROOM", ".prefab", ".PREFAB" };
	refresh("/assets");
	_root->clean();
}
ImporterMenu::~ImporterMenu()
{
	if (_root != nullptr)
		delete _root;
}
void ImporterMenu::render(bool &closeBool, Hydra::Renderer::Batch* previewBatch, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Once);
	ImGui::Begin("Import", &closeBool, ImGuiWindowFlags_MenuBar);
	_menuBar();

	Node* selectedNode = nullptr;

	//File tree
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, ImGui::GetWindowContentRegionMax().y - 60));
	if (_root != nullptr)
		_root->render(&selectedNode);
	if (selectedNode != nullptr)
	{
		if (selectedNode->openInFileExplorer)
		{
			openInExplorer(selectedNode->reverseEngineerPath());
			selectedNode->openInFileExplorer = false;
		}
		if (selectedNode->getExt() == ".mATTIC")
		{
			std::string fileName = selectedNode->reverseEngineerPath();
			previewBatch->objects.clear();
			// Because of issues with the engine, adding components will always be added to the global world
			// So we need to disable the meshes to have them be only viewed in the previewWindow
			_previewEntity = world::newEntity(fileName, world::invalidID); // invalidID - Will not show up in the entity list
			_previewEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(fileName);
			auto drawObject = _previewEntity->getComponent<Hydra::Component::DrawObjectComponent>();
			drawObject->drawObject->disable = true; // Disable mesh to only see the model in preview window
			auto t = _previewEntity->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3{ 0, 0, 10 };
			auto cc = _previewEntity->addComponent<Hydra::Component::CameraComponent>();
			cc->mouseControl = false;
			cc->noClip = false;
			previewBatch->pipeline->setValue(0, cc->getViewMatrix());
			previewBatch->pipeline->setValue(1, cc->getProjectionMatrix());
			previewBatch->pipeline->setValue(2, t->position);
			previewBatch->objects[drawObject->drawObject->mesh].push_back(drawObject->drawObject->modelMatrix);
		}
		std::string ext = selectedNode->getExt();
		if (ext == ".mattic" || ext == ".mATTIC")
		{
			if (ImGui::IsMouseDoubleClicked(0) && getRoomEntity() != nullptr)
			{
				std::shared_ptr<Hydra::World::Entity> newEntity = world::newEntity(selectedNode->name(), getRoomEntity().get());
				newEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(selectedNode->reverseEngineerPath());
				newEntity->addComponent<Hydra::Component::TransformComponent>();
			}
		}
		else if (ext == ".room" || ext == ".ROOM")
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (getRoomEntity() != nullptr)
				{
					getRoomEntity().get()->dead = true;
				}
				auto room = world::newEntity("Room", world::root());
				auto bp = BlueprintLoader::load(workingDir + "/" + selectedNode->reverseEngineerPath());
				bp->spawn(room);
			}
		}
		else if (ext == ".prefab" || ext == ".PREFAB")
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (getRoomEntity() == nullptr)
				{
					auto room = world::newEntity("Workspace", world::root());
					auto t = room->addComponent<Hydra::Component::TransformComponent>();
					auto r = room->addComponent<Hydra::Component::RoomComponent>();
				}
				auto room = world::newEntity("Prefab", getRoomEntity());
				auto bp = BlueprintLoader::load(workingDir + "/" + selectedNode->reverseEngineerPath());
				bp->spawn(room);
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	Hydra::IEngine::getInstance()->getRenderer()->render((*previewBatch));
	//Preview window
	ImGui::BeginChild("Preview", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.7f, ImGui::GetWindowContentRegionMax().y - 60));
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<Hydra::Renderer::IFramebuffer&>(*previewBatch->renderTarget)[0]->getID()), ImVec2(ImGui::GetWindowContentRegionWidth() * 1.2f, ImGui::GetWindowContentRegionMax().y - 15));
	ImGui::EndChild();
	ImGui::End();
}