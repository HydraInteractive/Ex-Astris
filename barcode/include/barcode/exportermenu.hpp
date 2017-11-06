#pragma once
#include <barcode/filemanager.hpp>
class ExporterMenu : public FileTree {
public:
	ExporterMenu();
	~ExporterMenu();

	void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f);
private:
	std::string _selectedPath;
	char _selectedFileName[128] = "";
	bool _prepExporting = false;
};
