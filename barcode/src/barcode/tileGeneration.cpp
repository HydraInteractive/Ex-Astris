#include <barcode/tileGeneration.hpp>
using world = Hydra::World::World;

TileGeneration::TileGeneration(std::string middleRoomPath) {
	_obtainRoomFiles();
	pathfindingMap = new bool*[WORLD_MAP_SIZE];
	for (int i = 0; i < WORLD_MAP_SIZE; i++)
	{
		pathfindingMap[i] = new bool[WORLD_MAP_SIZE];
	}
	_setUpMiddleRoom(middleRoomPath);
	_createMapRecursivly(glm::ivec2(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2));
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
	
	//Random the rooms vector each time we're about to spawn a new room
	_randomizeRooms();


	//Go through all doors in the room of the tile, create a random room
	//That fits with the door, go through that room and so on, until the next
	//you get to a "end room". Then go back and continue for next door

	//Check if yTilePos or xTilePos == 0. If this is true, limit the rooms it can create
	//or we will go otuside the tile grid
	
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
			if (roomC->door[SOUTH] == true) {
				if (_checkAdjacents(pos.x, pos.y + 1, roomC))
				{
					placed = true;
					auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
					t->position = _gridToWorld(pos.x, pos.y + 1);
					t->scale = glm::vec3(1, 1, 1);
					roomGrid[pos.x][pos.y + 1] = roomC;
					_insertPathFindingMap(glm::ivec2(pos.x, pos.y + 1));
					_spawnRandomizedEnemies(t);
					_createMapRecursivly(glm::ivec2(pos.x, pos.y + 1));
				}
				else
					loadedRoom->dead = true;
			}			
			else
				loadedRoom->dead = true;
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
			auto doorBlock = world::newEntity("DoorBlock", world::root());
			doorBlock->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BlockCube2.mATTIC");
			auto t = doorBlock->addComponent<Hydra::Component::TransformComponent>();
			t->position = _gridToWorld(pos.x, pos.y + 1);
			t->position.z -= 17;
			t->scale = glm::vec3(3, 3, 3);
			//t->position
		}
	}

	if (roomGrid[pos.x][pos.y]->door[WEST] && roomGrid[pos.x + 1][pos.y] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();

			if (roomC->door[EAST] == true) {
				if (_checkAdjacents(pos.x + 1, pos.y, roomC))
				{
					placed = true;
					auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
					t->position = _gridToWorld(pos.x + 1, pos.y);
					t->scale = glm::vec3(1, 1, 1);
					roomGrid[pos.x + 1][pos.y] = roomC;
					_insertPathFindingMap(glm::ivec2(pos.x + 1, pos.y));
					_spawnRandomizedEnemies(t);
					_createMapRecursivly(glm::ivec2(pos.x + 1, pos.y));
				}
				else
					loadedRoom->dead = true;
			}
			else
				loadedRoom->dead = true;
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
			auto doorBlock = world::newEntity("DoorBlock", world::root());
			doorBlock->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BlockCube2.mATTIC");
			auto t = doorBlock->addComponent<Hydra::Component::TransformComponent>();
			t->position = _gridToWorld(pos.x + 1, pos.y);
			t->position.x -= 17;
			t->position.y += 1.5;
			t->scale = glm::vec3(3, 3, 3);
			//t->position
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
			if (roomC->door[NORTH] == true) {
				if (_checkAdjacents(pos.x, pos.y - 1, roomC))
				{
					placed = true;
					auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
					t->position = _gridToWorld(pos.x, pos.y - 1);
					t->scale = glm::vec3(1, 1, 1);
					roomGrid[pos.x][pos.y - 1] = roomC;
					_insertPathFindingMap(glm::ivec2(pos.x, pos.y - 1));
					_spawnRandomizedEnemies(t);
					_createMapRecursivly(glm::ivec2(pos.x, pos.y - 1));
				}
				else
					loadedRoom->dead = true;
			}
			else
				loadedRoom->dead = true;
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
			auto doorBlock = world::newEntity("DoorBlock", world::root());
			doorBlock->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BlockCube2.mATTIC");
			auto t = doorBlock->addComponent<Hydra::Component::TransformComponent>();
			t->position = _gridToWorld(pos.x, pos.y - 1);
			t->position.z += 17;
			t->scale = glm::vec3(3, 3, 3);
		}
	}

	if (roomGrid[pos.x][pos.y]->door[EAST] && roomGrid[pos.x - 1][pos.y] == nullptr)
	{
		bool placed = false;
		//Load all rooms and see if any of them fits
		for (size_t i = 0; i < _roomFileNames.size() && placed == false && _roomCounter < maxRooms; i++) {

			//Take a random room and read it. Don't spawn it until it fits
			//NOTE:: Make a random list and go through it to prevent loading same room multible times
			std::shared_ptr<Hydra::World::Entity> loadedRoom = world::newEntity("Room", world::root());

			BlueprintLoader::load(_roomFileNames[i])->spawn(loadedRoom);
			auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();
			if (roomC->door[WEST] == true) {
				if (_checkAdjacents(pos.x - 1, pos.y, roomC))
				{
					placed = true;
					auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
					t->position = _gridToWorld(pos.x - 1, pos.y);
					t->scale = glm::vec3(1, 1, 1);
					roomGrid[pos.x - 1][pos.y] = roomC;
					_insertPathFindingMap(glm::ivec2(pos.x - 1, pos.y));
					_spawnRandomizedEnemies(t);
					_createMapRecursivly(glm::ivec2(pos.x - 1, pos.y));
				}
				else
					loadedRoom->dead = true;
			}
			else
				loadedRoom->dead = true;
		}
		//If for some reason no room at all fits, spawn a door/rubble/something
		if (placed == false) {
			//Place stuff to cover door
			auto doorBlock = world::newEntity("DoorBlock", world::root());
			doorBlock->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/BlockCube2.mATTIC");
			auto t = doorBlock->addComponent<Hydra::Component::TransformComponent>();
			t->position = _gridToWorld(pos.x - 1, pos.y);
			t->position.x += 17;
			t->scale = glm::vec3(3, 3, 3);
		}
	}
}

void TileGeneration::_setUpMiddleRoom(std::string middleRoomPath) {

	auto room = world::newEntity("Middle Room", world::root());
	BlueprintLoader::load(middleRoomPath)->spawn(room);
	auto roomC = room->getComponent<Hydra::Component::RoomComponent>();
	roomGrid[ROOM_GRID_SIZE / 2][ROOM_GRID_SIZE / 2] = roomC;
	_insertPathFindingMap(glm::ivec2(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2));
	auto t = room->addComponent<Hydra::Component::TransformComponent>();
	t->position = _gridToWorld(2, 2);
	t->scale = glm::vec3(1, 1, 1);
	localXY = glm::vec2(0, 0);
	
	//t->rotation = glm::quat(1, 0, 1, 0);
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
	std::string path = "assets/room/";
	for (auto & p : std::experimental::filesystem::directory_iterator(path)) {
		_roomFileNames.push_back(p.path().string());
	}

	_randomizeRooms();

}

void TileGeneration::_randomizeRooms() {

	//Randomize the list 2 times for extra randomness
	for (size_t k = 0; k < 2; k++) {
		for (size_t i = 0; i < _roomFileNames.size(); i++) {
			int randomPos = rand() % _roomFileNames.size();
			std::swap(_roomFileNames[i], _roomFileNames[randomPos]);
		}
	}
}

void TileGeneration::_spawnRandomizedEnemies(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform) {

	_spawnPickUps(roomTransform);

	int randomSlowAliens = rand() % int(MAX_ENEMIES);
	int randomRobots = rand() % int(MAX_ENEMIES - randomSlowAliens);
	int randomFastAliens = rand() % int(MAX_ENEMIES - randomRobots);

	for (int i = 0; i < randomSlowAliens; i++) {
		auto alienEntity = world::newEntity("SlowAlien1", world::root());
		alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
		auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		a->damage = 4;
		a->behaviour->originalRange = 4;
		a->radius = 1;

		auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 80;
		h->health = 80;

		auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 4.0f;

		auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position.x = roomTransform->position.x + i;
		t->position.y = 0;
		t->position.z = roomTransform->position.z + i;
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
			0, 0, 0.6f, 1.0f);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
	}

	for (int i = 0; i < randomFastAliens; i++) {
		auto alienEntity = world::newEntity("FastAlien1", world::root());
		alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienFastModel.mATTIC");
		auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		a->damage = 4;
		a->behaviour->originalRange = 4;
		a->radius = 1;

		auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 60;
		h->health = 60;

		auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 10.0f;

		auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position.x = roomTransform->position.x + i + 2;
		t->position.y = 0;
		t->position.z = roomTransform->position.z + i + 2;
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
			0, 0, 0.6f, 1.0f);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
	}

	//for (int i = 0; i < randomRobots; i++) {
	//	auto robotEntity = world::newEntity("Robot1", world::root());
	//	robotEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/RobotModel2.mATTIC");
	//	auto a = robotEntity->addComponent<Hydra::Component::AIComponent>();
	//	a->behaviour = std::make_shared<AlienBehaviour>(robotEntity);
	//	a->damage = 4;
	//	a->behaviour->originalRange = 4;
	//	a->radius = 5;		
	//
	//	auto h = robotEntity->addComponent<Hydra::Component::LifeComponent>();
	//	h->maxHP = 120;
	//	h->health = 120;
	//
	//	auto m = robotEntity->addComponent<Hydra::Component::MovementComponent>();
	//	m->movementSpeed = 2.0f;
	//	auto t = robotEntity->addComponent<Hydra::Component::TransformComponent>();
	//	t->position.x = roomTransform->position.x + i + 2;
	//	t->position.y = 0;
	//	t->position.z = roomTransform->position.z + i + 2;
	//	t->scale = glm::vec3{ 1,1,1 };
	//
	//	//t->rotation = glm::vec3{ 0, 90, 0 };
	//	auto rgbc = robotEntity->addComponent<Hydra::Component::RigidBodyComponent>();
	//	rgbc->createBox(glm::vec3(0.5f) * t->scale, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
	//		0, 0, 0.6f, 1.0f);
	//	rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
	//}

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

void TileGeneration::_spawnPickUps(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform) {

	int randomChance = rand() % 100 + 1;

	if (randomChance < (int)PICKUP_CHANCE) {
		auto pickUpEntity = world::newEntity("PickUp", world::root());
		auto t = pickUpEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position = glm::vec3(roomTransform->position.x, 0.0f, roomTransform->position.z);
		pickUpEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/GreenCargoBox.mATTIC");
		pickUpEntity->addComponent<Hydra::Component::PickUpComponent>();
		auto rgbc = pickUpEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(2.0f, 1.5f, 1.7f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PICKUP_OBJECT, 10);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
	}
}

glm::vec3 TileGeneration::_gridToWorld(int x, int y) {

	float xPos = (ROOM_SIZE * x) - ((ROOM_GRID_SIZE * ROOM_SIZE) / 2) + 17;
	float yPos = (ROOM_SIZE * y) - ((ROOM_GRID_SIZE * ROOM_SIZE) / 2) + 17;
	 
	glm::vec3 finalPos = glm::vec3(xPos, -7, yPos);
	return finalPos;

}

bool TileGeneration::_checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r) {
	if (r->door[r->NORTH])
	{
		if (y >= ROOM_GRID_SIZE - 1)
			return false;
		if (roomGrid[x][y + 1] != nullptr)
			if (roomGrid[x][y + 1]->door[r->SOUTH] == false)
				return false;
		
	}
	if (r->door[r->EAST])
	{
		if (x <= 0)
			return false;
		if (roomGrid[x - 1][y] != nullptr)
			if(roomGrid[x - 1][y]->door[r->WEST] == false)
				return false;
	}
	if (r->door[r->SOUTH])
	{
		if (y <= 0)
			return false;
		if (roomGrid[x][y - 1] != nullptr)
			if(roomGrid[x][y - 1]->door[r->NORTH] == false)
				return false;
	}
	if (r->door[r->WEST])
	{
		if (x >= ROOM_GRID_SIZE - 1)
			return false;
		if (roomGrid[x + 1][y] != nullptr)
			if(roomGrid[x + 1][y]->door[r->EAST] == false)
				return false;
	}
	return true;
}
