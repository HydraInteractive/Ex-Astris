#pragma once

#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/world/blueprintloader.hpp>
#include <hydra/physics/bulletmanager.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <imgui/imgui.h>
#include <barcode/ImporterMenu.hpp>
#include <barcode/ExporterMenu.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/EditorCameraComponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>

#include <hydra/io/input.hpp>

#include <fstream>
#include <json.hpp>

class tileGeneration
{
public:

	struct roomInfo {

		int nrOfTiles;
		int nrOfDoors;

		int roomID;

		bool downDoor = false;
		bool leftDoor = false;
		bool upDoor = false;
		bool rightDoor = false;

		int xPos;
		int yPos;

		std::vector<int> tileIndices;
		//Door indices for each room tile
		//int doorIndices[4];

		//std::vector<newMeshInfo*> realATTICMeshes;
	};

	struct tileInfo {

		glm::vec3 middlePoint;

		bool taken = false;

		int xTilePos;
		int yTilePos;

		glm::mat4 tileModelMatrix;
		roomInfo room;

	};
	std::vector<tileInfo*> tiles;

	tileGeneration(int xSize, int ySize);
	~tileGeneration();

private:
	int _xSize, _ySize;

	void _setupGrid();
	void _createMapRecursivly(tileInfo *tile);

};