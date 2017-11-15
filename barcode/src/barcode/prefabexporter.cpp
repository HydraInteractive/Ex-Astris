#include <barcode/prefabexporter.hpp>

PrefabExporter::PrefabExporter() : FileTree()
{
	this->_extWhitelist = { ".room", ".ROOM" };
	refresh("/assets");
}

PrefabExporter::~PrefabExporter()
{

}

void PrefabExporter::refresh(std::string path)
{
	FileTree::refresh(path);
}

void PrefabExporter::render(bool &closeBool, Hydra::Renderer::Batch* previewBatch, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Export", &closeBool, ImGuiWindowFlags_MenuBar);
	_menuBar();
	ImGui::End();
}