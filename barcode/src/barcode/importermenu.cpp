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
	this->_extWhitelist = {".mATTIC", ".mattic", ".room", ".ROOM"};
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
	_menuBar();

	Node* selectedNode = nullptr;

	//File tree
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, ImGui::GetWindowContentRegionMax().y - 60));
	if(_root != nullptr)
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

			auto cc = _previewEntity->addComponent<Hydra::Component::CameraComponent>();
			cc->position = glm::vec3{ 0, 0, 5 };
			previewBatch->pipeline->setValue(0, cc->getViewMatrix());
			previewBatch->pipeline->setValue(1, cc->getProjectionMatrix());
			previewBatch->pipeline->setValue(2, cc->position);
			previewBatch->objects[drawObject->drawObject->mesh].push_back(drawObject->drawObject->modelMatrix);
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

void ImporterMenu::Node::render(Node** selectedNode)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	auto label = ICON_FA_FOLDER " %s";
	if (ImGui::TreeNodeEx(this, nodeFlags, label, _name.c_str()))
	{
		label = ICON_FA_FOLDER_OPEN " %s";
		if (ImGui::IsItemClicked())
		{
			(*selectedNode) = this;
		}
		if (ImGui::BeginPopupContextItem(_name.c_str()))
		{
			ImGui::MenuItem("Show in File Explorer", "", &openInFileExplorer);
			(*selectedNode) = this;
			ImGui::EndPopup();
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
				ImGui::TreeNodeEx(_files[i], nodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBE " %s", _files[i]->_name.c_str());
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
				if (ImGui::BeginPopupContextItem(_files[i]->_name.c_str()))
				{
					ImGui::MenuItem("Show in File Explorer", "", &_files[i]->openInFileExplorer);
					(*selectedNode) = _files[i];
					ImGui::EndPopup();
				}
			}
			else if (ext == ".room" || ext == ".ROOM")
			{
				ImGui::TreeNodeEx(_files[i], nodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBES " %s", _files[i]->_name.c_str());
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
				if (ImGui::BeginPopupContextItem(_files[i]->_name.c_str()))
				{
					ImGui::MenuItem("Show in File Explorer", "", &_files[i]->openInFileExplorer);
					(*selectedNode) = _files[i];
					ImGui::EndPopup();
				}
			}
			else
			{
				ImGui::TreeNodeEx(_files[i], nodeFlags | ImGuiTreeNodeFlags_Leaf, ICON_FA_QUESTION_CIRCLE_O " %s", _files[i]->_name.c_str());
			}
		}
		ImGui::TreePop();
	}
}