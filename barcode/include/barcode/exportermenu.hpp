#pragma once
#include <barcode/filetree.hpp>
#include <hydra/world/world.hpp>
#include <hydra/ext/api.hpp>
class ExporterMenu : public FileTree {
public:
	ExporterMenu();
	~ExporterMenu();

	void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f);
	void refresh(std::string path = "/assets");
private:
	char _selectedFileName[128] = "";
	int _fileType = 0;
	int _fileTreeFileType = 0;
	static const int nrOfTypes = 2;
	const char* _exportTypes[nrOfTypes] = { ".room",".prefab" };
	bool _prepExporting = false;
	Hydra::World::Entity* selectedEntity = nullptr;

	void _renderEntitySelector();
	bool _overwritePopup(std::string fileToSave);
};
