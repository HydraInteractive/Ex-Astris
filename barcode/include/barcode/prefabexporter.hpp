#pragma once
#include <barcode/filetree.hpp>
class PrefabExporter : public FileTree
{
public:
	PrefabExporter();
	~PrefabExporter();
	void refresh(std::string path);
	void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f);
private:

};