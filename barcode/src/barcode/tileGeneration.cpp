#include <barcode/tileGeneration.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>


using world = Hydra::World::World;

TileGeneration::TileGeneration(std::string middleRoomPath) {
	_obtainRoomFiles();
	_setupGrid();
	_setUpMiddleRoom(middleRoomPath);
	_createMapRecursivly(glm::ivec2(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2));
	pathfindingMap = new bool*[WORLD_MAP_SIZE];
	for (int i = 0; i < WORLD_MAP_SIZE; i++)
	{
		pathfindingMap[i] = new bool[WORLD_MAP_SIZE];
	}
}

TileGeneration::~TileGeneration() {
	for (int i = 0; i < WORLD_MAP_SIZE; i++)
	{
		delete[] pathfindingMap[i];
	}
	delete[] pathfindingMap;
}

void TileGeneration::_createMapRecursivly(glm::ivec2 pos) {

	_roomCounter++;
	//Go through all doors in the room of the tile, create a random room
	//That fits with the door, go through that room and so on, until the next
	//you get to a "end room". Then go back and continue for next door

	//Check if yTilePos or xTilePos == 0. If this is true, limit the rooms it can create
	//or we will go otuside the tile roomGrid
	
	enum { NORTH, EAST, SOUTH, WEST };
	if (roomGrid[pos.x][pos.y]->door[NORTH] && roomGrid[pos.x][pos.y + 1] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			auto loadedRoom = world::newEntity("Room", world::root());
			
			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(pos.x, pos.y + 1, roomC))
			{
				placed = true;
				auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
				t->position = _gridToWorld(pos.x, pos.y + 1);
				roomGrid[pos.x][pos.y + 1] = roomC;
				_insertPathFindingMap(glm::ivec2(pos.x, pos.y + 1));
				_createMapRecursivly(glm::ivec2(pos.x, pos.y + 1));
			}
			else {
				loadedRoom->dead = true;
			}
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
		}
	}


	if (roomGrid[pos.x][pos.y]->door[EAST] && roomGrid[pos.x + 1][pos.y] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(pos.x + 1, pos.y, roomC))
			{
				placed = true;
				auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
				t->position = _gridToWorld(pos.x + 1, pos.y);
				roomGrid[pos.x + 1][pos.y] = roomC;
				_insertPathFindingMap(glm::ivec2(pos.x + 1, pos.y));
				_createMapRecursivly(glm::ivec2(pos.x + 1, pos.y));
			}
			else {
				loadedRoom->dead = true;
			}
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
		}
	}

	if (roomGrid[pos.x][pos.y]->door[SOUTH] && roomGrid[pos.x][pos.y - 1] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(pos.x, pos.y - 1, roomC))
			{
				placed = true;
				auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
				t->position = _gridToWorld(pos.x, pos.y - 1);
				roomGrid[pos.x][pos.y - 1] = roomC;
				_insertPathFindingMap(glm::ivec2(pos.x, pos.y - 1));
				_createMapRecursivly(glm::ivec2(pos.x, pos.y - 1));
			}
			else {
				loadedRoom->dead = true;
			}
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
		}
	}

	if (roomGrid[pos.x][pos.y]->door[WEST] && roomGrid[pos.x - 1][pos.y] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(pos.x - 1, pos.y, roomC))
			{
				placed = true;
				auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
				t->position = _gridToWorld(pos.x - 1, pos.y);
				roomGrid[pos.x - 1][pos.y] = roomC;
				_insertPathFindingMap(glm::ivec2(pos.x - 1, pos.y));
				_createMapRecursivly(glm::ivec2(pos.x - 1, pos.y));
			}
			else {
				loadedRoom->dead = true;
			}
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
		}
	}
}

void TileGeneration::_insertPathFindingMap(glm::ivec2 room)
{
	auto roomC = roomGrid[room.x][room.y];
	int x = room.x * ROOM_MAP_SIZE;
	for (int localX = 0; localX < ROOM_MAP_SIZE; x++, localX++)
	{
		int y = room.y * ROOM_MAP_SIZE;
		for (int localY = 0; localY < ROOM_MAP_SIZE; y++, localY++)
		{
			pathfindingMap[x][y] = roomC->localMap[localX][localY];
		}
	}
}

void TileGeneration::_setUpMiddleRoom(std::string middleRoomPath) {

	auto room = world::newEntity("Middle Room", world::root());
	auto t = room->addComponent<Hydra::Component::TransformComponent>();
	t->position = glm::vec3(0, -5, 0);
	auto roomC = room->addComponent<Hydra::Component::RoomComponent>();
	roomGrid[ROOM_GRID_SIZE / 2][ROOM_GRID_SIZE / 2] = roomC;
	BlueprintLoader::load(middleRoomPath)->spawn(room);
}

void TileGeneration::_obtainRoomFiles() {

	//Get the files in order
	std::string path = "assets/room/";
	//for (auto & p : std::experimental::filesystem::directory_iterator(path)) {
	//_roomFileNames.push_back(p.path().string());
	//}
	_roomFileNames.push_back(path + "starterRoom.room");
	_roomFileNames.push_back(path + "threewayRoom.room");

	//Randomize the list 3 times for extra randomness
	for (size_t k = 0; k < 3; k++) {
		for (size_t i = 0; i < _roomFileNames.size(); i++) {
			int randomPos = rand() % _roomFileNames.size();
			std::swap(_roomFileNames[i], _roomFileNames[randomPos]);
		}
	}

}

glm::vec3 TileGeneration::_gridToWorld(int x, int y) {

	float xPos = (ROOM_SIZE * x) - ((ROOM_GRID_SIZE * ROOM_SIZE) / 2) + 17;
	float yPos = (ROOM_SIZE * y) - ((ROOM_GRID_SIZE * ROOM_SIZE) / 2) + 17;

	return glm::vec3(xPos, -5, yPos);
	
}

bool TileGeneration::_checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r) {
	if (r->door[r->NORTH])
	{
		if (y >= ROOM_GRID_SIZE)
			return false;
		if (roomGrid[x][y + 1] != nullptr && !roomGrid[x][y + 1]->door[r->SOUTH])
			return false;
	}
	if (r->door[r->WEST])
	{
		if (x <= 0)
			return false;
		if (roomGrid[x - 1][y] != nullptr && !roomGrid[x - 1][y]->door[r->EAST])
			return false;
	}
	if (r->door[r->SOUTH])
	{
		if (y <= 0)
			return false;
		if (roomGrid[x][y - 1] != nullptr && !roomGrid[x][y - 1]->door[r->NORTH])
			return false;
	}
	if (r->door[r->EAST])
	{
		if (x >= ROOM_GRID_SIZE)
			return false;
		if (roomGrid[x + 1][y] != nullptr && !roomGrid[x + 1][y]->door[r->WEST])
			return false;
	}
	return true;
}

void TileGeneration::_setupGrid() {
	int idCount = 0;
	//Get the middle tile
	//If the tiles are for example 4x4 roomGrid, there is no exact middle
	//Then we cannot subtract with 1
	//if ((_xSize * _ySize) % 2 == 0) {
	//	middleTile = ((_xSize * _ySize)) / 2;
	//}
	//else {
	////Otherwise there is a middle point. We subtract with 1 to get the
	////tiles in array order
	//	middleTile = ((_xSize * _ySize) - 1) / 2;
	//}

	/*for (size_t y = 0; y < _ySize; y++) {

		for (size_t x = 0; x < _xSize; x++) {

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
	*/
}
