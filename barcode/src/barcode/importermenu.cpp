#include <barcode/importermenu.hpp>
#include <hydra/engine.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

ImporterMenu::ImporterMenu() : FileTree()
{
	this->executableDir = _getExecutableDir();
	this->_root = nullptr;
	this->_newEntityClicked = true;
	this->extWhitelist = {".mATTIC", ".mattic", ".room", ".ROOM"};
	refresh("/assets");
	_root->clean();
}
ImporterMenu::~ImporterMenu()
{
	if(_root != nullptr)
	delete _root;
}
void ImporterMenu::render(bool &closeBool, Hydra::Renderer::Batch* previewBatch, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Import", &closeBool, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Refresh", NULL))
			{
				refresh("/assets");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	Node* selectedNode = nullptr;

	//File tree
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, ImGui::GetWindowContentRegionMax().y - 60));
	if(_root != nullptr)
		_root->render(&selectedNode);
	if (selectedNode != nullptr && selectedNode->getExt() == ".mATTIC") {
		std::string fileName = selectedNode->reverseEngineerPath();
		std::cout << fileName.c_str() << std::endl;

		if (previewBatch->objects.size() > 0) {
			if (_previewEntity->name.c_str() == fileName.c_str()) // FIXME: Wtf?
				_newEntityClicked = false;
			else
				_newEntityClicked = true;
		}

		previewBatch->objects.clear();
		// Because of issues with the engine adding components will always be added to the global world
		// So we need to disable the meshes to have them be only viewed in the previewWindow
		if (_newEntityClicked && selectedNode->getExt() == ".mATTIC") {
			_previewEntity = world::newEntity(fileName, world::invalidID); // invalidID - Will not show up in the entity list
			_previewEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(fileName);
			auto drawObject = _previewEntity->getComponent<Hydra::Component::DrawObjectComponent>();
			drawObject->drawObject->disable = true; // Disable mesh to only see the model in preview window

			auto cc = _previewEntity->addComponent<Hydra::Component::CameraComponent>();
			cc->position = glm::vec3{0, 0, 5};
		}
		auto drawObject = _previewEntity->getComponent<Hydra::Component::DrawObjectComponent>();
		auto cc = _previewEntity->getComponent<Hydra::Component::CameraComponent>();
		previewBatch->pipeline->setValue(0, cc->getViewMatrix());
		previewBatch->pipeline->setValue(1, cc->getProjectionMatrix());
		previewBatch->pipeline->setValue(2, cc->position);
		previewBatch->objects[drawObject->drawObject->mesh].push_back(drawObject->drawObject->modelMatrix);
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

void ImporterMenu::Node::render(Node** selectedNode)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	auto label = ICON_FA_FOLDER " %s";
	if (ImGui::TreeNodeEx(this, node_flags, label, _name.c_str()))
	{
		label = ICON_FA_FOLDER_OPEN " %s";
		if (ImGui::IsItemClicked())
		{
			(*selectedNode) = this;
		}
		for (size_t i = 0; i < this->_subfolders.size(); i++)
		{
			_subfolders[i]->render(selectedNode);
		}
		for (size_t i = 0; i < this->_files.size(); i++)
		{
			std::string ext = this->_files[i]->getExt();
			if (ext == ".mattic" || ext == ".mATTIC")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBE " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked())
				{
					(*selectedNode) = _files[i];
					if (ImGui::IsMouseDoubleClicked(0) && getRoomEntity() != nullptr)
					{
						std::shared_ptr<Hydra::World::Entity> newEntity = world::newEntity(_files[i]->name(), getRoomEntity().get());
						newEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(_files[i]->reverseEngineerPath());
						newEntity->addComponent<Hydra::Component::TransformComponent>();
					}
				}
			}
			else if (ext == ".room" || ext == ".ROOM")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBES " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked())
				{
					(*selectedNode) = _files[i];
					if (ImGui::IsMouseDoubleClicked(0))
					{
						if (getRoomEntity() != nullptr)
						{
							getRoomEntity().get()->dead = true;
						}
						auto room = world::newEntity("Room", world::root());
						auto bp = BlueprintLoader::load(_files[i]->reverseEngineerPath());
						bp->spawn(room);
					}
				}
			}
			else
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_QUESTION_CIRCLE_O " %s", _files[i]->_name.c_str());
			}
		}
		ImGui::TreePop();
	}
}
