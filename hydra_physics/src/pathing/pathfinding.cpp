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

bool PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos)
{
	if (map == nullptr)
		return false;

	openList.clear();
	visitedList.clear();
	pathToEnd.clear();

	MapVec mapCurrentPos = worldToMapCoords(currentPos);
	MapVec mapTargetPos = worldToMapCoords(targetPos);

	if (isOutsideMap(mapCurrentPos.baseVec) || isOutsideMap(mapTargetPos.baseVec))
		return false;

	if (inWall(currentPos))
	{
		currentPos = findViableTile(currentPos);
	}

	if (targetPos.y < 4.5f) {
		if (inWall(targetPos))
		{
			printf("PLAYER IN WALL!\n");
			targetPos = findViableTile(targetPos);
		}
	}

	mapCurrentPos = worldToMapCoords(currentPos);
	mapTargetPos = worldToMapCoords(targetPos);
	//If either position is out of bounds, abort
	if (isOutOfBounds(mapCurrentPos.baseVec) || isOutOfBounds(mapTargetPos.baseVec))
		return false;

	//If the player is in line of sight don't path just go straight
	//if (_inLineOfSight(mapCurrentPos,mapTargetPos))
	//	pathToEnd.push_back(targetPos);
	//	return false;
	_startNode = new Node(mapCurrentPos.x(), mapCurrentPos.z(), nullptr);
	_endNode = new Node(mapTargetPos.x(), mapTargetPos.z(), nullptr);

	_startNode->H = _startNode->hDistanceTo(_endNode);
	openList.push_back(_startNode);

	foundGoal = false;

	while (!openList.empty() && !foundGoal)
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
	return MapVec((worldPos.x * ROOM_SCALE) - ROOM_GRID_SIZE, (worldPos.z * ROOM_SCALE) - ROOM_GRID_SIZE);
}

glm::vec3 PathFinding::mapToWorldCoords(const MapVec& mapPos)
{
	return glm::vec3((mapPos.baseVec.x + ROOM_GRID_SIZE) / ROOM_SCALE, 0, (mapPos.baseVec.y + ROOM_GRID_SIZE) / ROOM_SCALE);
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
	if (map[vec.x][vec.y] == 0)
	{
		return true;
	}
	return false;
}
bool PathFinding::isOutsideMap(const glm::ivec2 & vec) const
{
	if (vec.x >= WORLD_MAP_SIZE || vec.y >= WORLD_MAP_SIZE || vec.x < 0 || vec.y < 0)
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
	glm::ivec2& vec = p.baseVec;

	if (vec.x >= WORLD_MAP_SIZE || vec.y >= WORLD_MAP_SIZE || vec.x < 0 || vec.y < 0)
	{
		return false;
	}
	if (map[vec.x][vec.y] == 0)
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
	//for (int i = 1; i < 3; i++)
	//{
	//	if (map[vec.x + i][vec.y] == 1)
	//	{
	//		newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z);
	//	}
	//	else if (map[vec.x][vec.y + i] == 1)
	//	{
	//		newPos = glm::vec3(mapPos.x, mapPos.y, mapPos.z + i);
	//	}
	//	else if (map[vec.x - i][vec.y] == 1)
	//	{
	//		newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z);
	//	}
	//	else if (map[vec.x][vec.y - i] == 1)
	//	{
	//		newPos = glm::vec3(mapPos.x + 1, mapPos.y, mapPos.z - i);
	//	}
	//	else if (map[vec.x + i][vec.y + i] == 1)
	//	{
	//		newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z + i);
	//	}
	//	else if (map[vec.x - i][vec.y + i])
	//	{
	//		newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z + i);
	//	}
	//	else if (map[vec.x - i][vec.y - i])
	//	{
	//		newPos = glm::vec3(mapPos.x - i, mapPos.y, mapPos.z - i);
	//	}
	//	else if (map[vec.x + i][vec.y - i])
	//	{
	//		newPos = glm::vec3(mapPos.x + i, mapPos.y, mapPos.z - i);
	//	}
	//}

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

static int sortFunc(void const* aPtr, void const* bPtr) {
	const std::shared_ptr<PathFinding::Node>& a = *static_cast<const std::shared_ptr<PathFinding::Node>*>(aPtr);
	const std::shared_ptr<PathFinding::Node>& b = *static_cast<const std::shared_ptr<PathFinding::Node>*>(bPtr);
	return a->getF() > b->getF();
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
	//std::qsort(openList.data(), openList.size(), sizeof(openList.data()[0]), sortFunc);
}