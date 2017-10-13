#include <barcode/tileGeneration.hpp>


tileGeneration::tileGeneration(int xSize, int ySize)
{

	_xSize = xSize;
	_ySize = ySize;

	_setupGrid();
	
}

tileGeneration::~tileGeneration()
{
}

void tileGeneration::_createMapRecursivly(tileInfo *tile) {

	for (int i = 0; i < tile->room.nrOfDoors; i++) {

		//Go through all doors in the room of the tile, create a random room
		//That fits with the door, go through that room and so on, until the next
		//you get to a "end room". Then go back and continue for next door

		if (tile->room.downDoor == 1 && tile->yTilePos >= 0) {
			//Find the tile under the current tile
			for (int k = 0; k < tiles.size(); k++) {

				if (!tiles[k]->taken) {
					if (tile->yTilePos == tiles[k]->yTilePos && tile->xTilePos == tiles[k]->xTilePos - 1) {
						//Got em

						//Set up fitting room
						roomInfo *rom = new roomInfo;

						//readATTICRoom("Models/Rooms/leftUpInfo.ATTIC", "Models/Rooms/leftUpModel.ATTIC", *rom);
						tiles[k]->room = *rom;

						//call function again from new tile
						_createMapRecursivly(tiles[k]);
					}
				}
			}
		}
		if (tile->room.leftDoor == 1 && tile->xTilePos >= 0) {
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->xTilePos == tiles[k]->xTilePos - 1 && tile->yTilePos == tiles[k]->yTilePos) {
						roomInfo *rom = new roomInfo;

						//readATTICRoom("Models/Rooms/leftUpRoomInfo.ATTIC", "Models/Rooms/leftUpRoomModel.ATTIC", *rom);
						tiles[k]->room = *rom;

						//call function again from new tile
						_createMapRecursivly(tiles[k]);
					}
				}
			}
		}
		if (tile->room.upDoor == 1 && tile->yTilePos <= 4) {
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->xTilePos == tiles[k]->xTilePos && tile->yTilePos == tiles[k]->yTilePos + 1) {
						roomInfo *rom = new roomInfo;

						//readATTICRoom("Models/Rooms/leftUpRoomInfo.ATTIC", "Models/Rooms/leftUpRoomModel.ATTIC", *rom);
						tiles[k]->room = *rom;

						//call function again from new tile
						_createMapRecursivly(tiles[k]);
					}
				}
			}
		}
		if (tile->room.rightDoor == 1 && tile->xTilePos <= 4) {
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->xTilePos == tiles[k]->xTilePos + 1 && tile->yTilePos == tiles[k]->yTilePos) {
						roomInfo *rom = new roomInfo;

						//readATTICRoom("Models/Rooms/leftUpRoomInfo.ATTIC", "Models/Rooms/leftUpRoomModel.ATTIC", *rom);
						tiles[k]->room = *rom;

						//call function again from new tile
						_createMapRecursivly(tiles[k]);
					}
				}
			}
		}
	}
}


void tileGeneration::_setupGrid() {

	int tileCount = 0;
	int positiveXCount = 0;
	int positiveYCount = 0;
	int middleTile;
	//Get the middle tile
	//If the tiles are for example 4x4 grid, there is no exact middle
	//Then we cannot subtract with 1
	if ((_xSize * _ySize) % 2 == 0) {
		middleTile = ((_xSize * _ySize)) / 2;
	}
	//Otherwise there is a middle point. We subtract with 1 to get the
	//tiles in array order
	else {
		middleTile = ((_xSize * _ySize) - 1) / 2;
	}

	for (int y = 0; y < _ySize; y++) {

		positiveXCount = 0;
		positiveYCount = 0;

		for (int x = 0; x < _xSize; x++) {

			tileInfo *tile = new tileInfo;

			float xPos = 34 * (x + 1) - 17;
			float yPos = 34 * (y + 1) - 17;

			//float xPos;
			//float yPos;
			//int xTilePosition = (xSize - (x + 1));
			//int yTilePosition = (ySize - (y + 1));
			//
			//if (xTilePosition > 0) {
			//	xPos = xTilePosition *(-17);
			//	//xPos += 17;
			//}
			//else {
			//	xPos = (abs(xTilePosition) + 1) * 17;
			//	xPos += 17;
			//}
			//if (yTilePosition > 0) {
			//	yPos = yTilePosition * - 17;
			//	//yPos += 17;
			//}
			//else {
			//	yPos = (abs(yTilePosition) + 1) * 17;
			//	yPos += 17;
			//}

			tile->middlePoint = glm::vec3(xPos + 17, 0, yPos + 17);

			tile->tileModelMatrix = glm::rotate(glm::mat4(), glm::radians(360.0f), glm::vec3(0, 1, 0));
			tile->tileModelMatrix = glm::scale(glm::vec3(1));
			tile->tileModelMatrix[3] = glm::vec4(tile->middlePoint, 1);

			//tile->room = *rom;

			tile->xTilePos = x;
			tile->yTilePos = y;
			tiles.push_back(tile);

			tileCount++;

			delete[] tile;

		}
	}

	_createMapRecursivly(tiles[middleTile]);
	//bool si = true;

}