// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* A* pathfinding
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/pathing/pathfinding.hpp>
std::shared_ptr<Hydra::Component::RoomComponent> PathFinding::roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
PathFinding::PathFinding() {
	openList = std::vector<Node*>();
	visitedList = std::vector<Node*>();
	foundGoal = true;
}

PathFinding::~PathFinding() {
	for (auto&& node : openList)
	{
		delete node;
	}
	for (auto&& node : visitedList)
	{
		delete node;
	}
	delete _endNode;
}

bool PathFinding::prePathfinding(MapVec origin, MapVec target)
{
	for (int i = 0; i < ROOM_GRID_SIZE; i++)
	{
		for (int j = 0; j < ROOM_GRID_SIZE; j++)
		{
			roomPathMap[i][j] = false;
		}
	}
	roomOpenList.clear();
	roomVisitedList.clear();
	origin.baseVec = origin.baseVec / ROOM_MAP_SIZE;
	target.baseVec = target.baseVec / ROOM_MAP_SIZE;

	if (origin.x() >= ROOM_GRID_SIZE || origin.z() >= ROOM_GRID_SIZE || origin.x() < 0 || origin.z() < 0)
	{
		return false;
	}
	if (target.x() >= ROOM_GRID_SIZE || target.z() >= ROOM_GRID_SIZE || target.x() < 0 || target.z() < 0)
	{
		return false;
	}

	originNode = new Node(origin.x(), origin.z(), nullptr);
	targetNode = new Node(target.x(), target.z(), nullptr);

	originNode->H = originNode->hDistanceTo(targetNode);
	roomOpenList.push_back(originNode);

	enum { NORTH, EAST, SOUTH, WEST };
	while (!roomOpenList.empty())
	{
		Node* currentNode = roomOpenList.back();
		roomVisitedList.push_back(roomOpenList.back());
		roomOpenList.pop_back();

		//End reached
		if (currentNode->pos == targetNode->pos)
		{
			targetNode->lastNode = currentNode->lastNode;

			Node* getPath = targetNode;

			while (getPath != nullptr)
			{
				roomPathMap[getPath->pos.x()][getPath->pos.z()] = true;
				getPath = getPath->lastNode;
				
			}
			return true;
		}
		//Navigate map
		else
		{
			if (roomGrid[currentNode->pos.x()][currentNode->pos.z()] == nullptr)
			{
				continue;
			}
			//East
			if(roomGrid[currentNode->pos.x()][currentNode->pos.z()]->door[EAST])
				_discoverPrePathNode(currentNode->pos.x() + 1, currentNode->pos.z(), currentNode, WEST);
			//West
			if (roomGrid[currentNode->pos.x()][currentNode->pos.z()]->door[WEST])
				_discoverPrePathNode(currentNode->pos.x() - 1, currentNode->pos.z(), currentNode, EAST);
			//North
			if (roomGrid[currentNode->pos.x()][currentNode->pos.z()]->door[NORTH])
				_discoverPrePathNode(currentNode->pos.x(), currentNode->pos.z() - 1, currentNode, SOUTH);
			//South
			if (roomGrid[currentNode->pos.x()][currentNode->pos.z()]->door[SOUTH])
				_discoverPrePathNode(currentNode->pos.x(), currentNode->pos.z() + 1, currentNode, NORTH);
		}
	}
	return false;
}
void PathFinding::_discoverPrePathNode(int x, int z, Node* lastNode, size_t oppositeDir)
{
	MapVec currentPos = MapVec(x, z);
	if (currentPos.x() >= ROOM_GRID_SIZE || currentPos.z() >= ROOM_GRID_SIZE || currentPos.x() < 0 || currentPos.z() < 0)
	{
		return;
	}

	if (roomGrid[currentPos.x()][currentPos.z()] == nullptr)
	{
		return;
	}
	//If the room isn't open in this direction, don't go to it
	if (!roomGrid[currentPos.x()][currentPos.z()]->door[oppositeDir])
	{
		return;
	}

	//If the node has already been visited, don't worry about it
	for (size_t i = 0; i < roomVisitedList.size(); i++)
	{
		if (currentPos == roomVisitedList[i]->pos)
		{
			return;
		}
	}
	Node* thisNode = nullptr;

	//If this node exists in the open list, don't add it again
	for (size_t i = 0; i < roomOpenList.size() && thisNode == nullptr; i++)
	{
		if (currentPos == roomOpenList[i]->pos)
		{
			thisNode = roomOpenList[i];
		}
	}
	//This node hasn't been found before, add it to the open list
	if (thisNode == nullptr)
	{
		thisNode = new Node(x, z, lastNode);
		thisNode->G = INFINITY;
		thisNode->H = INFINITY;
		roomOpenList.push_back(thisNode);
	}

	//Check if this node has had a better path to it before
	float thisPathF = lastNode->G + lastNode->gDistanceTo(thisNode) + thisNode->hDistanceTo(targetNode);
	//If this is a better path than previously, replace the old path values
	if (thisNode->getF() > thisPathF)
	{
		thisNode->G = lastNode->G + lastNode->gDistanceTo(thisNode);
		thisNode->H = thisNode->hDistanceTo(targetNode);
		thisNode->lastNode = lastNode;
	}

	std::sort(roomOpenList.begin(), roomOpenList.end(), comparisonFunctor);
}
bool PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos)
{
	if (map == nullptr)
		return false;

	openList.clear();
	visitedList.clear();
	pathToEnd.clear();

	MapVec mapCurrentPos = worldToMapCoords(currentPos);
	MapVec mapTargetPos = worldToMapCoords(targetPos);

	if (inWall(currentPos))
	{
		currentPos = findViableTile(currentPos);
	}

	if (targetPos.y < 4.5f) {
		if (inWall(targetPos))
		{
			targetPos = findViableTile(targetPos);
		}
	}

	mapCurrentPos = worldToMapCoords(currentPos);
	mapTargetPos = worldToMapCoords(targetPos);
	
	if (!prePathfinding(mapCurrentPos, mapTargetPos))
	{
		std::cout << "WARNING NO PREPATHING PATH FOUND, IS AI OUTSIDE OF MAP?" << std::endl;
	}
	//If either position is out of bounds, abort
	if (isOutOfBounds(mapCurrentPos.baseVec) || isOutOfBounds(mapTargetPos.baseVec))
		return false;

	_startNode = new Node(mapCurrentPos.x(), mapCurrentPos.z(), nullptr);
	_endNode = new Node(mapTargetPos.x(), mapTargetPos.z(), nullptr);

	_startNode->H = _startNode->hDistanceTo(_endNode);
	openList.push_back(_startNode);

	foundGoal = false;


	while (!openList.empty() && !foundGoal && visitedList.size() < 1000)
	{
		Node* currentNode = openList.back();
		visitedList.push_back(openList.back());
		openList.pop_back();

		//End reached
		if (currentNode->pos == _endNode->pos)
		{
			_endNode->lastNode = currentNode->lastNode;

			Node* getPath = _endNode;

			while (getPath != nullptr)
			{
				pathToEnd.push_back(mapToWorldCoords(getPath->pos));
				getPath = getPath->lastNode;
			}
			foundGoal = true;
			return true;
		}
		//Navigate map
		else
		{
			//East
			_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z(), currentNode);
			//West
			_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z(), currentNode);
			//North
			_discoverNode(currentNode->pos.x(), currentNode->pos.z() + 1, currentNode);
			//South
			_discoverNode(currentNode->pos.x(), currentNode->pos.z() - 1, currentNode);
			//North West
			_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z() + 1, currentNode);
			//North East
			_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z() + 1, currentNode);
			//South West
			_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z() - 1, currentNode);
			//South East
			_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z() - 1, currentNode);
		}
	}
	return false;
}

PathFinding::MapVec PathFinding::worldToMapCoords(const glm::vec3& worldPos)
{
	return MapVec(worldPos.x * ROOM_SCALE, worldPos.z * ROOM_SCALE);
}

glm::vec3 PathFinding::mapToWorldCoords(const MapVec& mapPos)
{
	return glm::vec3(mapPos.baseVec.x / ROOM_SCALE, 0, mapPos.baseVec.y / ROOM_SCALE);
}

bool PathFinding::isOutOfBounds(const glm::ivec2& vec) const
{
	if (vec.x >= WORLD_MAP_SIZE || vec.y >= WORLD_MAP_SIZE || vec.x < 0 || vec.y < 0)
	{
		return true;
	}
	if (map == nullptr)
	{
		std::cout << "ERROR: NO PATHFINDING MAP\n";
		return true;
	}
	if (map[vec.x][vec.y] == false)
	{
		return true;
	}
	//std::cout << vec.x / ROOM_MAP_SIZE << " " << vec.y / ROOM_MAP_SIZE << std::endl;
	if (roomPathMap[vec.x/ROOM_MAP_SIZE][vec.y/ROOM_MAP_SIZE] == false)
	{
		return true;
	}
	return false;
}
bool PathFinding::inLineOfSight(const glm::vec3& enemyPos, const glm::vec3& playerPos) const
{
	MapVec e = worldToMapCoords(enemyPos);
	MapVec p = worldToMapCoords(playerPos);
	return _inLineOfSight(e, p);
}

bool PathFinding::inWall(const glm::vec3 mapPos) const
{
	MapVec p = worldToMapCoords(mapPos);

	if (p.x() >= WORLD_MAP_SIZE || p.z() >= WORLD_MAP_SIZE || p.x() < 0 || p.z() < 0)
	{
		return false;
	}
	if (map[p.x()][p.z()] == 0)
	{
		return true;
	}
	return false;
}

glm::vec3 PathFinding::findViableTile(glm::vec3 mapPos) const
{
	// This might crash because of no out of bounds check
	// Does not work with an out of bounds check
	MapVec p = worldToMapCoords(mapPos);
	glm::ivec2& vec = p.baseVec;

	glm::vec3 newPos = mapPos;
	for (int i = 0; i < 3; i++) {
		if ((vec.x + i < WORLD_MAP_SIZE && vec.y < WORLD_MAP_SIZE && vec.x + i >= 0 && vec.y >= 0) && map[vec.x + i][vec.y] == 1)
			newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z);
		else if ((vec.x < WORLD_MAP_SIZE && vec.y + i < WORLD_MAP_SIZE && vec.x >= 0 && vec.y + i >= 0) && map[vec.x][vec.y + i] == 1)
			newPos = glm::vec3(mapPos.x, mapPos.y, mapPos.z + i);
		else if ((vec.x - i < WORLD_MAP_SIZE && vec.y < WORLD_MAP_SIZE && vec.x - i >= 0 && vec.y >= 0) && map[vec.x - i][vec.y] == 1)
			newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z);
		else if ((vec.x < WORLD_MAP_SIZE && vec.y - i < WORLD_MAP_SIZE && vec.x >= 0 && vec.y - i >= 0) && map[vec.x][vec.y - i] == 1)
			newPos = glm::vec3(mapPos.x + 1, mapPos.y, mapPos.z - i);
		else if ((vec.x + i < WORLD_MAP_SIZE && vec.y + i < WORLD_MAP_SIZE && vec.x + i >= 0 && vec.y + i >= 0) && map[vec.x + i][vec.y + i] == 1)
			newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z + i);
		else if ((vec.x - i < WORLD_MAP_SIZE && vec.y + i < WORLD_MAP_SIZE && vec.x - i >= 0 && vec.y + i >= 0) && map[vec.x - i][vec.y + i] == 1)
			newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z + i);
		else if ((vec.x - i < WORLD_MAP_SIZE && vec.y - i < WORLD_MAP_SIZE && vec.x - i >= 0 && vec.y - i >= 0) && map[vec.x - i][vec.y - i] == 1)
			newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z - i);
		else if ((vec.x + i < WORLD_MAP_SIZE && vec.y - i < WORLD_MAP_SIZE && vec.x + i >= 0 && vec.y - i >= 0) && map[vec.x + i][vec.y - i] == 1)
			newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z - i);
	}

	return newPos;
}

void PathFinding::setRoomGrid(std::shared_ptr<Hydra::Component::RoomComponent> newRoomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE])
{
	for (int i = 0; i < ROOM_GRID_SIZE; i++)
	{
		for (int j = 0; j < ROOM_GRID_SIZE; j++)
		{
			roomGrid[i][j] = newRoomGrid[i][j];
		}
	}
}

bool PathFinding::_inLineOfSight(const MapVec enemyPos, const MapVec playerPos) const
{
	glm::vec2 dir = playerPos.baseVec - enemyPos.baseVec;
	float distance = dir.length();
	dir = glm::normalize(dir);
	glm::vec2 currentPos = enemyPos;
	for (int i = 0; i < distance; i++)
	{
		if (isOutOfBounds(currentPos))
		{
			return false;
		}
		currentPos += dir;
	}
	return true;
}

void PathFinding::_discoverNode(int x, int z, Node* lastNode)
{
	MapVec currentPos = MapVec(x, z);
	if (isOutOfBounds(currentPos.baseVec))
	{
		return;
	}

	//If the node has already been visited, don't worry about it
	for (size_t i = 0; i < visitedList.size(); i++)
	{
		if (currentPos == visitedList[i]->pos)
		{
			return;
		}
	}
	Node* thisNode = nullptr;

	//If this node exists in the open list, don't add it again
	for (size_t i = 0; i < openList.size() && thisNode == nullptr; i++)
	{
		if (currentPos == openList[i]->pos)
		{
			thisNode = openList[i];
		}
	}
	//This node hasn't been found before, add it to the open list
	if (thisNode == nullptr)
	{
		thisNode = new Node(x, z, lastNode);
		thisNode->G = INFINITY;
		thisNode->H = INFINITY;
		openList.push_back(thisNode);
	}

	//Check if this node has had a better path to it before
	float thisPathF = lastNode->G + lastNode->gDistanceTo(thisNode) + thisNode->hDistanceTo(_endNode);
	//If this is a better path than previously, replace the old path values
	if (thisNode->getF() > thisPathF)
	{
		thisNode->G = lastNode->G + lastNode->gDistanceTo(thisNode);
		thisNode->H = thisNode->hDistanceTo(_endNode);
		thisNode->lastNode = lastNode;
	}

	std::sort(openList.begin(), openList.end(), comparisonFunctor);
}