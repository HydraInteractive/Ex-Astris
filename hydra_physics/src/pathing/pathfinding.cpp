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
	intializedStartGoal = false;
	foundGoal = false;
}

PathFinding::~PathFinding(){

}

void PathFinding::findPath(const glm::vec3& currentPos, const glm::vec3& targetPos, int (&map)[MAP_SIZE][MAP_SIZE])
{
	if (!intializedStartGoal) 
	{
		_openList.clear();
		_visitedList.clear();
		_pathToEnd.clear();
		
		_startNode = std::make_shared<Node>(currentPos.x / NODE_SCALE, currentPos.z / NODE_SCALE, nullptr);
		_endNode = std::make_shared<Node>(targetPos.x / NODE_SCALE, targetPos.z / NODE_SCALE, nullptr);

		_startNode->H = _startNode->hDistanceTo(_endNode);
		_openList.push_back(_startNode);

		foundGoal = false;
		intializedStartGoal = true;
	}
	if (!foundGoal)
	{
		//TODO: Infinite loop concerns
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
					_pathToEnd.push_back(MapVec(getPath->pos.x() * NODE_SCALE, getPath->pos.z() * NODE_SCALE));
					getPath = getPath->lastNode;
				}
				foundGoal = true;
			}
			//Navigate map
			else
			{
				//East
				_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z(), currentNode, map);

				//West
				_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z(), currentNode, map);

				//North
				_discoverNode(currentNode->pos.x(), currentNode->pos.z() + 1, currentNode, map);

				//South
				_discoverNode(currentNode->pos.x(), currentNode->pos.z() - 1, currentNode, map);

				//North West
				_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z() + 1, currentNode, map);

				//North East
				_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z() + 1, currentNode, map);

				//South West
				_discoverNode(currentNode->pos.x() - 1, currentNode->pos.z() - 1, currentNode, map);

				//South East
				_discoverNode(currentNode->pos.x() + 1, currentNode->pos.z() - 1, currentNode, map);
			}
		}
	}
}

glm::vec3 PathFinding::nextPathPos(const glm::vec3& pos, const float& radius)
{
	glm::vec3 nextPos = _pathToEnd.back();
	nextPos.x += (NODE_SCALE / 2);
	nextPos.z += (NODE_SCALE / 2);
	
	float distance = glm::distance(pos, nextPos);
	if (!_pathToEnd.empty())
	{
		if (distance < radius)
		{
			_pathToEnd.pop_back();
		}
	}
	return nextPos;
}

void PathFinding::_discoverNode(int x, int z, std::shared_ptr<Node> lastNode, int(&map)[MAP_SIZE][MAP_SIZE])
{
	//If this node is inaccessable, ignore it
	if (map[x][z] == 1 || map[x][z] == 2)
	{
		return;
	}

	MapVec currentPos = MapVec(x, z);

	//If the node has already been visited, don't worry about itas
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
	for (size_t i = 0; i < _openList.size() && !found;i++)
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
