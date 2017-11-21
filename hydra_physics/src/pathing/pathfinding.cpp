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
	_openList = std::vector<std::shared_ptr<Node>>();
	_visitedList = std::vector<std::shared_ptr<Node>>();
	foundGoal = true;
}

PathFinding::~PathFinding() {

}

bool PathFinding::findPath(const glm::vec3& currentPos, const glm::vec3& targetPos)
{
	if (map == nullptr)
		return false;

	_openList.clear();
	_visitedList.clear();
	pathToEnd.clear();

	MapVec mapCurrentPos = worldToMapCoords(currentPos);
	MapVec mapTargetPos = worldToMapCoords(targetPos);
	//If either position is out of bounds, abort
	if (isOutOfBounds(mapCurrentPos.baseVec) || isOutOfBounds(mapTargetPos.baseVec))
		return false;
	//If the player is in line of sight don't path just go straight
	//if (_inLineOfSight(mapCurrentPos,mapTargetPos))
	//	pathToEnd.push_back(targetPos);
	//	return false;
	_startNode = std::make_shared<Node>(mapCurrentPos.x(), mapCurrentPos.z(), nullptr);
	_endNode = std::make_shared<Node>(mapTargetPos.x(), mapTargetPos.z(), nullptr);

	_startNode->H = _startNode->hDistanceTo(_endNode);
	_openList.push_back(_startNode);

	foundGoal = false;

	while (!_openList.empty() && !foundGoal)
	{
		std::shared_ptr<Node> currentNode = _openList.back();
		_visitedList.push_back(_openList.back());
		_openList.pop_back();

		//End reached
		if (currentNode->pos == _endNode->pos)
		{
			_endNode->lastNode = currentNode->lastNode;

			std::shared_ptr<Node> getPath = _endNode;

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
PathFinding::MapVec PathFinding::worldToMapCoords(const glm::vec3& worldPos) const
{
	return MapVec((worldPos.x / MAP_SCALE) + (MAP_SIZE / 2), (worldPos.z / MAP_SCALE) + (MAP_SIZE / 2));
}

PathFinding::MapVec PathFinding::worldToMapCoords(const glm::vec3& worldPos) const
{
	return MapVec((worldPos.x / ROOM_SCALE) + (WORLD_MAP_SIZE / 2), (worldPos.z / ROOM_SCALE) + (WORLD_MAP_SIZE / 2));
}

glm::vec3 PathFinding::mapToWorldCoords(const MapVec& mapPos) const
{
	return glm::vec3((mapPos.baseVec.x - (WORLD_MAP_SIZE / 2)) * ROOM_SCALE, 0, (mapPos.baseVec.y - (WORLD_MAP_SIZE / 2)) * ROOM_SCALE);
}

bool PathFinding::isOutOfBounds(const glm::ivec2& vec) const
{
	if (vec.x > WORLD_MAP_SIZE || vec.y > WORLD_MAP_SIZE || vec.x < 0 || vec.y < 0)
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
bool PathFinding::inLineOfSight(const glm::vec3 enemyPos, const glm::vec3 playerPos) const
{
	MapVec e = worldToMapCoords(enemyPos);
	MapVec p = worldToMapCoords(playerPos);
	return _inLineOfSight(e, p);
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

void PathFinding::_discoverNode(int x, int z, std::shared_ptr<Node> lastNode)
{
	MapVec currentPos = MapVec(x, z);
	if (isOutOfBounds(currentPos.baseVec))
	{
		return;
	}

	//If the node has already been visited, don't worry about it
	for (size_t i = 0; i < _visitedList.size(); i++)
	{
		if (currentPos == _visitedList[i]->pos)
		{
			return;
		}
	}
	std::shared_ptr<Node> thisNode;

	//If this node exists in the open list, don't add it again
	bool found = false;
	for (size_t i = 0; i < _openList.size() && !found; i++)
	{
		if (currentPos == _openList[i]->pos)
		{
			found = true;
			thisNode = _openList[i];
		}
	}
	//This node hasn't been found before, add it to the open list
	if (!found)
	{
		thisNode = std::make_shared<Node>(x, z, lastNode);
		thisNode->G = INFINITY;
		thisNode->H = INFINITY;
		_openList.push_back(thisNode);
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
	std::sort(_openList.begin(), _openList.end(), comparisonFunctor);
}
