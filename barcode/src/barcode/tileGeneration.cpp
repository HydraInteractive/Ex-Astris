#include <barcode/tileGeneration.hpp>
using world = Hydra::World::World;

TileGeneration::TileGeneration(std::string middleRoomPath) {
	_obtainRoomFiles();
	_setupGrid();
	setUpMiddleRoom(middleRoomPath);
	//_createMapRecursivly(tiles[middleTile].get());
}

TileGeneration::~TileGeneration() {}

void TileGeneration::_createMapRecursivly(glm::ivec2 pos) {

	//Go through all doors in the room of the tile, create a random room
	//That fits with the door, go through that room and so on, until the next
	//you get to a "end room". Then go back and continue for next door

	//Check if yTilePos or xTilePos == 0. If this is true, limit the rooms it can create
	//or we will go otuside the tile grid
	bool placed = false;
	enum { NORTH, EAST, SOUTH, WEST };
	if (grid[pos.x][pos.y]->door[NORTH])
	{
		//Load all rooms and see if any of them fits
		for (int i = 0; i < _roomFileNames.size() && placed == false; i++) {

			//std::string roomString = "Tile ";
			//roomString += std::to_string(tiles[k].get()->tileID);

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(glm::ivec2(pos.x, pos.y + 1), roomC))
			{
				placed = true;
				_createMapRecursivly(glm::ivec2(pos.x, pos.y + 1));
			}
			//if (roomC->door[roomC->NORTH]) {
			//	//If the tile is at the end of the grid or at the corner of the
			//	//dont spawn a room downards outside the grid
			//	if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == _xSize - 1) {
			//		//Only spawn a room wih and updoor and a leftdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == 0) {
			//		//Only spawn a room wih and updoor and a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0) {
			//		//Only spawn a room wih and updoor, a leftdoor and/or a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else {
			//		//Room can spawn with any doors, as long at it has a door that
			//		//fits the last tile (Up)
			//		fits = true;
			//	}
			//}
			//else
			//	loadedRoom->dead = true;
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
	}
	//If for some reason no room at all fits, spawn a door/rubble/something
	if (placed == false) {
		//Place stuff to cover door
	}

	if (grid[pos.x][pos.y]->door[EAST])
	{
		//Load all rooms and see if any of them fits
		for (int i = 0; i < _roomFileNames.size() && placed == false; i++) {

			//std::string roomString = "Tile ";
			//roomString += std::to_string(tiles[k].get()->tileID);

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(glm::ivec2(pos.x + 1, pos.y), roomC))
			{
				placed = true;
				_createMapRecursivly(glm::ivec2(pos.x + 1, pos.y));
			}
			//if (roomC->door[roomC->NORTH]) {
			//	//If the tile is at the end of the grid or at the corner of the
			//	//dont spawn a room downards outside the grid
			//	if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == _xSize - 1) {
			//		//Only spawn a room wih and updoor and a leftdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == 0) {
			//		//Only spawn a room wih and updoor and a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0) {
			//		//Only spawn a room wih and updoor, a leftdoor and/or a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else {
			//		//Room can spawn with any doors, as long at it has a door that
			//		//fits the last tile (Up)
			//		fits = true;
			//	}
			//}
			//else
			//	loadedRoom->dead = true;
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
	}
	//If for some reason no room at all fits, spawn a door/rubble/something
	if (placed == false) {
		//Place stuff to cover door
	}

	if (grid[pos.x][pos.y]->door[SOUTH])
	{
		//Load all rooms and see if any of them fits
		for (int i = 0; i < _roomFileNames.size() && placed == false; i++) {

			//std::string roomString = "Tile ";
			//roomString += std::to_string(tiles[k].get()->tileID);

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(glm::ivec2(pos.x, pos.y - 1), roomC))
			{
				placed = true;
				_createMapRecursivly(glm::ivec2(pos.x, pos.y - 1));
			}
			//if (roomC->door[roomC->NORTH]) {
			//	//If the tile is at the end of the grid or at the corner of the
			//	//dont spawn a room downards outside the grid
			//	if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == _xSize - 1) {
			//		//Only spawn a room wih and updoor and a leftdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == 0) {
			//		//Only spawn a room wih and updoor and a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0) {
			//		//Only spawn a room wih and updoor, a leftdoor and/or a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else {
			//		//Room can spawn with any doors, as long at it has a door that
			//		//fits the last tile (Up)
			//		fits = true;
			//	}
			//}
			//else
			//	loadedRoom->dead = true;
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
	}
	//If for some reason no room at all fits, spawn a door/rubble/something
	if (placed == false) {
		//Place stuff to cover door
	}

	if (grid[pos.x][pos.y]->door[WEST])
	{
		//Load all rooms and see if any of them fits
		for (int i = 0; i < _roomFileNames.size() && placed == false; i++) {

			//std::string roomString = "Tile ";
			//roomString += std::to_string(tiles[k].get()->tileID);

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (_checkAdjacents(glm::ivec2(pos.x - 1, pos.y), roomC))
			{
				placed = true;
				_createMapRecursivly(glm::ivec2(pos.x - 1, pos.y));
			}
			//if (roomC->door[roomC->NORTH]) {
			//	//If the tile is at the end of the grid or at the corner of the
			//	//dont spawn a room downards outside the grid
			//	if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == _xSize - 1) {
			//		//Only spawn a room wih and updoor and a leftdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0 && tiles[k]->xTilePos == 0) {
			//		//Only spawn a room wih and updoor and a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else if (tiles[k]->yTilePos == 0) {
			//		//Only spawn a room wih and updoor, a leftdoor and/or a rightdoor.
			//		//Or an 1 door room (end room)
			//		fits = true;
			//	}
			//	else {
			//		//Room can spawn with any doors, as long at it has a door that
			//		//fits the last tile (Up)
			//		fits = true;
			//	}
			//}
			//else
			//	loadedRoom->dead = true;
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
	}
	//If for some reason no room at all fits, spawn a door/rubble/something
	if (placed == false) {
		//Place stuff to cover door
	}
}


void _spawnRoomEntity(Hydra::World::Entity* ) {

}

void TileGeneration::setUpMiddleRoom(std::string middleRoomPath) {

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

void TileGeneration::_obtainRoomFiles() {

	//Get the files in order
	std::string path = "assets/rooms/";
	for (auto & p : std::experimental::filesystem::directory_iterator(path)) {
		_roomFileNames.push_back(p.path().string());
	}

	//Randomize the list 3 times for extra randomness
	for (int k = 0; k < 3; k++) {
		for (int i = 0; i < _roomFileNames.size(); i++) {
			int randomPos = rand() % _roomFileNames.size();
			std::swap(_roomFileNames[i], _roomFileNames[randomPos]);
		}
	}

}

bool TileGeneration::_checkAdjacents(glm::ivec2 pos, std::shared_ptr<Hydra::Component::RoomComponent>& r) {
	if (r->door[r->NORTH])
	{
		if (pos.y >= GRID_SIZE)
			return false;
		if (grid[pos.x][pos.y + 1] == nullptr || !grid[pos.x][pos.y + 1]->door[r->SOUTH])
			return false;
	}
	if (r->door[r->WEST])
	{
		if (pos.x <= 0)
			return false;
		if (grid[pos.x - 1][pos.y] == nullptr || !grid[pos.x - 1][pos.y]->door[r->EAST])
			return false;
	}
	if (r->door[r->SOUTH])
	{
		if (pos.y <= 0)
			return false;
		if (grid[pos.x][pos.y - 1] == nullptr || !grid[pos.x][pos.y - 1]->door[r->NORTH])
			return false;
	}
	if (r->door[r->EAST])
	{
		if (pos.x >= GRID_SIZE)
			return false;
		if (grid[pos.x + 1][pos.y] == nullptr || !grid[pos.x + 1][pos.y]->door[r->WEST])
			return false;
	}
	return true;
}

void TileGeneration::_setupGrid() {
	int idCount = 0;
	//Get the middle tile
	//If the tiles are for example 4x4 grid, there is no exact middle
	//Then we cannot subtract with 1
	//if ((_xSize * _ySize) % 2 == 0) {
	//	middleTile = ((_xSize * _ySize)) / 2;
	//}
	//else {
	////Otherwise there is a middle point. We subtract with 1 to get the
	////tiles in array order
	//	middleTile = ((_xSize * _ySize) - 1) / 2;
	//}

	/*for (int y = 0; y < _ySize; y++) {

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
	*/
}
