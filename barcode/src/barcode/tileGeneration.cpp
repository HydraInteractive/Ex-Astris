#include <barcode/tileGeneration.hpp>
using world = Hydra::World::World;

tileGeneration::tileGeneration(int xSize, int ySize, std::string middleRoomPath ) {
	_xSize = xSize;
	_ySize = ySize;
	_obtainRoomFiles();
	_setupGrid();
	setUpMiddleRoom(middleRoomPath);
}

tileGeneration::~tileGeneration() {}

void tileGeneration::_createMapRecursivly(tileInfo *tile) {

	bool downDoorChecked = false;
	bool leftDoorChecked = false;
	bool upDoorChecked = false;
	bool rightDoorChecked = false;

	for (int i = 0; i < tile->room.nrOfDoors; i++) {

		//Go through all doors in the room of the tile, create a random room
		//That fits with the door, go through that room and so on, until the next
		//you get to a "end room". Then go back and continue for next door

		//Check if yTilePos or xTilePos == 0. If this is true, limit the rooms it can create
		//or we will go otuside the tile grid
		bool fits = false;

		if (tile->room.downDoor == 1 && tile->yTilePos >= 0 && downDoorChecked == false) {
			//Find the tile under the current tile
			downDoorChecked = true;
			for (int k = 0; k < tiles.size() && fits == false; k++) {

				if (!tiles[k]->taken) {
					if (tile->tileID - _ySize == tiles[k]->tileID) {
						//For all loaded rooms
						while (fits == false) {

							//Set up fitting room
							//roomInfo *room = new roomInfo;

							std::string roomString = "Tile ";
							roomString += std::to_string(tiles[k].get()->tileID);

							//Take a random room and read it. Don't spawn it until it fits
							//NOTE:: Make a random list and go through it to prevent loading same room multible times
							std::string roomFile = _roomFileNames[std::rand() % _roomFileNames.size()];
							std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

							BlueprintLoader::load(roomFile)->spawn(loadedRoom);
							auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();
							
							_setDoors()

							if (roomC->door[roomC->NORTH]) {
								//If the tile is at the end of the grid or at the corner of the
								//dont spawn a room downards outside the grid
								if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == _xSize - 1) {
									//Only spawn a room wih and updoor and a leftdoor.
									//Or an 1 door room (end room)
									fits = true;
								}
								else if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == 0) {
									//Only spawn a room wih and updoor and a rightdoor.
									//Or an 1 door room (end room)
									fits = true;
								}
								else if (tiles[k]->yTilePos == 0) {
									//Only spawn a room wih and updoor, a leftdoor and/or a rightdoor.
									//Or an 1 door room (end room)
									fits = true;
								}
								else {
									//Room can spawn with any doors, as long at it has a door that
									//fits the last tile (Up)
									fits = true;
								}
							}
							else 
								loadedRoom->dead = true;
							

							//Load in a room to either see if it fits, or that the
							//file name knows if it fits
							//Have a if-stament to see so that the room doesn't
							//have a door to a closed wall/map end
							//if true, fits = true;
							//else, load in new room and try again
							//Apply the mesh/entity to the room with
							//right coordinates
							//tiles[k]->room = *room;
							//delete room;

						}
						//call function again from new tile
						tile->taken = true;
						_createMapRecursivly(tiles[k].get());
					}
				}
			}
		}
		if (tile->room.leftDoor == 1 && tile->xTilePos >= 0 && leftDoorChecked == false) {
			leftDoorChecked = true;
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->tileID - 1 == tiles[k]->tileID) {
						while (fits == false) {

							std::string roomString = "Tile";
							roomString += std::to_string(tiles[k].get()->tileID);

							//Take a random room and read it. Don't spawn it until it fits
							std::string roomFile = _roomFileNames[std::rand() % _roomFileNames.size()];
							std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

							BlueprintLoader::load(roomFile)->spawn(loadedRoom);
							auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

							if (roomC->door[roomC->EAST]) {
								//If the tile is at the end of the grid or at the corner
								//dont spawn a room downards outside the grid
								if (tiles[k]->xTilePos == 0 && tiles[k]->yTilePos == _ySize - 1) {
									//Only spawn a room wih and downDoor and a rightDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->xTilePos == 0 && tiles[k]->yTilePos == 0) {
									//Only spawn a room wih and upDoor and a rightDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->xTilePos == 0) {
									//Only spawn a room wih and rightDoor, a upDoor and/or a downDoor.
									//Or an 1 door room (end room)
								}
								else {
									//Room can spawn with any doors, as long at it has a door that
									//fits the last tile (Up)
									auto roomTile = world->createEntity(roomString.c_str());

									fits = true;
								}
							}
						}
						//call function again from new tile
						tile->taken = true;
						_createMapRecursivly(tiles[k].get(), world);
					}
				}
			}
		}
		if (tile->room.upDoor == 1 && tile->yTilePos <= _ySize && upDoorChecked == false) {
			upDoorChecked = true;
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->tileID + _xSize == tiles[k]->tileID) {
						while (fits == false) {

							std::string roomString = "Tile";
							roomString += std::to_string(tiles[k].get()->tileID);

							//Take a random room and read it. Don't spawn it until it fits
							std::string roomFile = _roomFileNames[std::rand() % _roomFileNames.size()];
							auto loadedRoom = world->getWorldRoot()->spawn(BlueprintLoader::load(roomFile.c_str())->spawn(world));

							if (loadedRoom.get()->getComponent<Hydra::Component::RoomComponent>()->getDownDoor()) {
								//If the tile is at the end of the grid or at the corner
								//dont spawn a room downards outside the grid
								if (tiles[k]->yTilePos == _ySize - 1 && tiles[k]->xTilePos == _xSize - 1) {
									//Only spawn a room wih and downDoor and a leftDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->yTilePos == _ySize - 1 && tiles[k]->xTilePos == 0) {
									//Only spawn a room wih and downDoor and a rightDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->yTilePos == _ySize - 1) {
									//Only spawn a room wih and downDoor, a rightDoor and/or a leftDoor.
									//Or an 1 door room (end room)
								}
								else {
									//Room can spawn with any doors, as long at it has a door that
									//fits the last tile (Up)
									auto roomTile = world->createEntity(roomString.c_str());

									fits = true;
								}
							}
						}
						//call function again from new tile
						tile->taken = true;
						_createMapRecursivly(tiles[k].get(), world);
					}
				}
			}
		}
		if (tile->room.rightDoor == 1 && tile->xTilePos <= _xSize && rightDoorChecked == false) {
			rightDoorChecked = true;
			for (int k = 0; k < tiles.size(); k++) {
				if (!tiles[k]->taken) {
					if (tile->tileID + 1 == tiles[k]->tileID) {
						while (fits == false) {

							std::string roomString = "Tile";
							roomString += std::to_string(tiles[k].get()->tileID);

							//Take a random room and read it. Don't spawn it until it fits
							std::string roomFile = _roomFileNames[std::rand() % _roomFileNames.size()];
							auto loadedRoom = world->getWorldRoot()->spawn(BlueprintLoader::load(roomFile.c_str())->spawn(world));

							if (loadedRoom.get()->getComponent<Hydra::Component::RoomComponent>()->getLeftDoor()) {
								//If the tile is at the end of the grid or at the corner
								//dont spawn a room downards outside the grid
								if (tiles[k]->xTilePos == _xSize - 1 && tiles[k]->yTilePos == _ySize - 1) {
									//Only spawn a room wih and downDoor and a leftDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->xTilePos == _xSize - 1 && tiles[k]->xTilePos == 0) {
									//Only spawn a room wih and leftDoor and a upDoor.
									//Or an 1 door room (end room)
								}
								else if (tiles[k]->xTilePos == _xSize - 1) {
									//Only spawn a room wih and leftDoor, a upDoor and/or a downDoor.
									//Or an 1 door room (end room)
								}
								else {
									//Room can spawn with any doors, as long at it has a door that
									//fits the last tile (Up)
									auto roomTile = world->createEntity(roomString.c_str());

									fits = true;
								}
							}
						}
						//call function again from new tile
						tile->taken = true;
						createMapRecursivly(tiles[k].get(), world);
					}
				}
			}
		}
	}
}

void _spawnRoomEntity(Hydra::World::Entity* ) {

}

void tileGeneration::setUpMiddleRoom(std::string middleRoomPath) {

	tiles[middleTile].get()->room.downDoor = true;
	tiles[middleTile].get()->room.upDoor = true;
	tiles[middleTile].get()->room.leftDoor = true;
	tiles[middleTile].get()->room.rightDoor = true;
	tiles[middleTile].get()->room.nrOfDoors = 4;


	auto room = world::newEntity("Middle Room", world::root());
	auto t = room->addComponent<Hydra::Component::TransformComponent>();
//	t->position = 
	BlueprintLoader::load(middleRoomPath)->spawn(room);

	//auto loadedRoom = world->getWorldRoot()->spawn(BlueprintLoader::load(middleRoomPath.c_str())->spawn(world));
	//auto middleRoomTile = world->createEntity("MiddleRoom");
	//middleRoomTile = loadedRoom;
	//middleRoomTile.get()->addComponent<Hydra::Component::MeshComponent>(loadedRoom.get()->getDrawObject()[0].mesh);
	//middleRoomTile.get()->addComponent<Hydra::Component::MeshComponent>(loadedRoom.get()->getDrawObject()[0].mesh);

	//middleRoomTile.get()->addComponent<Hydra::Component::TransformComponent>(tiles[middleTile].get()->middlePoint);
	//newEntity.get()->markDead();
}

void tileGeneration::_obtainRoomFiles() {

	std::string path = "assets/rooms/";
	for (auto & p : std::experimental::filesystem::directory_iterator(path)) {
		_roomFileNames.push_back(p.path().string());
	}

}

void tileGeneration::_setDoors(glm::ivec2 pos, std::shared_ptr<Hydra::Component::RoomComponent> r) {
	if (pos.x <= 0 || pos.y <= 0 || pos.x >= _xSize || pos.y >= _ySize)
	{

	}
	

}

void tileGeneration::_setupGrid() {
	int idCount = 0;
	//Get the middle tile
	//If the tiles are for example 4x4 grid, there is no exact middle
	//Then we cannot subtract with 1
	if ((_xSize * _ySize) % 2 == 0) {
		middleTile = ((_xSize * _ySize)) / 2;
	}
	else {
	//Otherwise there is a middle point. We subtract with 1 to get the
	//tiles in array order
		middleTile = ((_xSize * _ySize) - 1) / 2;
	}

	for (int y = 0; y < _ySize; y++) {

		for (int x = 0; x < _xSize; x++) {

			std::shared_ptr<tileInfo> tile = std::make_shared<tileInfo>();

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

			//tile->middlePoint = glm::vec3(17 * (x + 1) - (17 * _xSize), 0, (17 * (y + 1)) - (17 * _ySize));
			tile->middlePoint = glm::vec3(xPos, 0, yPos);

			tile->tileModelMatrix = glm::rotate(glm::mat4(), glm::radians(360.0f), glm::vec3(0, 1, 0));
			tile->tileModelMatrix = glm::scale(glm::vec3(1));
			tile->tileModelMatrix[3][0] = tile->middlePoint.x;
			tile->tileModelMatrix[3][1] = tile->middlePoint.y;
			tile->tileModelMatrix[3][2] = tile->middlePoint.z;

			//tile->room = *rom;
			tile->tileID = idCount;
			tile->xTilePos = x;
			tile->yTilePos = y;
			tiles.push_back(tile);
			idCount++;
		}
	}
}
