#include <barcode/exportermenu.hpp>
#include <hydra/component/roomcomponent.hpp>

using world = Hydra::World::World; 
ExporterMenu::ExporterMenu() : FileTree()
{
	this->_extWhitelist = { ".room", ".ROOM" };
	refresh("/assets");
}
ExporterMenu::~ExporterMenu()
{
	
}
void ExporterMenu::render(bool &closeBool, Hydra::Renderer::Batch* previewBatch, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Export", &closeBool, ImGuiWindowFlags_MenuBar);
	_menuBar();

	Node* selectedNode = nullptr;

	//File tree
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetWindowContentRegionMax().y - 60));
	if (_root != nullptr)
		_root->render(&selectedNode, _prepExporting);
	ImGui::EndChild();

	ImGui::SameLine();

	//File name dialog
	ImGui::BeginChild("File", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetWindowContentRegionMax().y - 60));

	//Refresh changes from file tree
	if (selectedNode != nullptr)
	{
		if (selectedNode->openInFileExplorer)
		{
			openInExplorer(selectedNode->reverseEngineerPath());
			selectedNode->openInFileExplorer = false;
		}
		_selectedPath = selectedNode->reverseEngineerPath();
		if (selectedNode->isAllowedFile)
		{
			_selectedPath.erase(_selectedPath.end() - selectedNode->name().length(), _selectedPath.end());

			std::string fileNameWithoutExt = selectedNode->name();
			fileNameWithoutExt.erase(fileNameWithoutExt.end() - selectedNode->getExt().length(), fileNameWithoutExt.end());
			//Force empty the string before copying the new one
			memset(_selectedFileName, 0, sizeof(_selectedFileName));
			strncpy(_selectedFileName, fileNameWithoutExt.c_str(), fileNameWithoutExt.length());
		}
		else
		{
			_selectedPath.append("/");
		}
	}

	//Draw gui
	ImGui::Text("Path: "); ImGui::SameLine(); ImGui::Text("%s", _selectedPath.c_str());
	ImGui::InputText(".room", _selectedFileName, 128);

	if (ImGui::Button("Save"))
	{
		_prepExporting = true;
	}
	if (_prepExporting)
	{
		std::string fileToSave = "";
		fileToSave.append(_selectedPath);
		fileToSave.append(_selectedFileName);
		fileToSave.append(".room");
		std::ifstream infile(fileToSave);
		bool doExport = false;
		if (infile.good())
		{
			ImGui::OpenPopup("Overwrite?");
		}
		else
		{
			doExport = true;
		}

		if (ImGui::BeginPopupModal("Overwrite?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string textline = "This action will overwrite the file at " + fileToSave + ". Continue?";
			ImGui::Text("%s", textline.c_str());
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				doExport = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{	
				_prepExporting = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (doExport)
		{
			std::ofstream outFile;
			if (getRoomEntity() != nullptr)
				BlueprintLoader::save(fileToSave, "Room", getRoomEntity());
			_prepExporting = false;
			closeBool = false;
		}
	}
	ImGui::EndChild();
	ImGui::End();
}
void ExporterMenu::Node::render(Node** selectedNode, bool& prepExporting)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	//TODO: Folder icon opening
	auto label = ICON_FA_FOLDER " %s";
	if (ImGui::TreeNodeEx(this, node_flags, label, _name.c_str()))
	{
		label = ICON_FA_FOLDER_OPEN " %s";
		if (ImGui::BeginPopupContextItem(_name.c_str()))
		{
			ImGui::MenuItem("Show in File Explorer", "", &openInFileExplorer);
			(*selectedNode) = this;
			ImGui::EndPopup();
		}
		if (ImGui::IsItemClicked())
		{
			(*selectedNode) = this;
		}
		for (size_t i = 0; i < this->_subfolders.size(); i++)
		{
			_subfolders[i]->render(selectedNode, prepExporting);
		}
		for (size_t i = 0; i < this->_files.size(); i++)
		{
			std::string ext = this->_files[i]->getExt();
			if (ext == ".mattic" || ext == ".mATTIC")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_CUBE " %s", _files[i]->_name.c_str());
				if (ImGui::BeginPopupContextItem(_files[i]->_name.c_str()))
				{
					ImGui::MenuItem("Show in File Explorer", "", &_files[i]->openInFileExplorer);
					(*selectedNode) = _files[i];
					ImGui::EndPopup();
				}
			}
			else if (ext == ".room" || ext == ".ROOM")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_CUBES " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
				{
					prepExporting = true;
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
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_QUESTION_CIRCLE_O " %s", _files[i]->_name.c_str());
				if (ImGui::BeginPopupContextItem(_files[i]->_name.c_str()))
				{
					ImGui::MenuItem("Show in File Explorer", "", &_files[i]->openInFileExplorer);
					(*selectedNode) = _files[i];
					ImGui::EndPopup();
				}
			}
			if (ImGui::IsItemClicked())
			{
				(*selectedNode) = _files[i];
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
