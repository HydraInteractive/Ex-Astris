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
	clearOpenList();
	intializedStartGoal = false;
	foundGoal = false;
}

PathFinding::~PathFinding(){

}

void PathFinding::findPath(const glm::vec3& currentPos, const glm::vec3& targetPos, int (&map)[WORLD_SIZE][WORLD_SIZE])
{
	if (!intializedStartGoal) 
	{
		clearOpenList();
		_visitedList.clear();
		_pathToEnd.clear();
		
		_startCell = std::make_shared<Node>(currentPos.x / CELL_SIZE, currentPos.z / CELL_SIZE, nullptr);
		_endCell = std::make_shared<Node>(targetPos.x / CELL_SIZE, targetPos.z / CELL_SIZE, nullptr);

		_startCell->H = _startCell->actualDistance(_endCell);
		_openList.push(_startCell);

		foundGoal = false;
		intializedStartGoal = true;
	}
	if (!foundGoal)
	{
		//TODO: Infinite loop concerns
		while(!_openList.empty() && !foundGoal)
		{
			std::shared_ptr<Node> currentCell = _openList.top();
			_visitedList.push_back(_openList.top());
			_openList.pop();

			//End reached
			if (currentCell->id == _endCell->id)
			{
				_endCell->lastNode = currentCell->lastNode;

				std::shared_ptr<Node> getPath = _endCell;

				while (getPath != nullptr)
				{
					_pathToEnd.push_back(MapVec(getPath->pos.x() * CELL_SIZE, getPath->pos.z() * CELL_SIZE));
					getPath = getPath->lastNode;
				}
				foundGoal = true;
			}
			//Navigate map
			else
			{
				//East
				_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z(), currentCell, map);
			
				//West
				_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z(), currentCell, map);
			
				//North
				_discoverNode(currentCell->pos.x(), currentCell->pos.z() + 1, currentCell, map);
			
				//South
				_discoverNode(currentCell->pos.x(), currentCell->pos.z() - 1, currentCell, map);
			
				//North West
				_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z() + 1, currentCell, map);
			
				//North East
				_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z() + 1, currentCell, map);
			
				//South West
				_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z() - 1, currentCell, map);
			
				//South East
				_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z() - 1, currentCell, map);
			}
		}

	}
}

glm::vec3& PathFinding::nextPathPos(const glm::vec3& pos, const float& radius)
{
	glm::vec3 nextPos = _pathToEnd.back();
	nextPos.x += (CELL_SIZE / 2);
	nextPos.z += (CELL_SIZE / 2);
	
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

void PathFinding::_discoverNode(int x, int z, std::shared_ptr<Node> lastNode, int(&map)[WORLD_SIZE][WORLD_SIZE])
{
	//If this node is inaccessable, ignore it
	if (map[x][z] == 1 || map[x][z] == 2)
	{
		return;
	}

	int id = z * WORLD_SIZE + x;

	//If the node has already been visited, don't add it again
	for (size_t i = 0; i < _visitedList.size(); i++)
	{
		if (id == _visitedList[i]->id)
		{
			return;
		}
	}
	std::shared_ptr<Node> thisNode;
	PriorityQueue tempQueue = _openList;

	//If this node exists in the open list don't add it again
	bool found = false;
	while (!tempQueue.empty() && !found)
	{
		if (id == tempQueue.top()->id)
		{
			found = true;
			thisNode = tempQueue.top();
		}
		tempQueue.pop();
	}
	if (!found)
	{
		thisNode = std::make_shared<Node>(x, z, lastNode);
		thisNode->G = INFINITY;
		thisNode->H = INFINITY;
		_openList.push(thisNode);
	}

	//Check if this node has had a better path to it before
	float distanceViaLastNode = lastNode->G + lastNode->actualDistance(thisNode);
	//If this is the best path, replace the old path values
	if (thisNode->getF() > distanceViaLastNode)
	{
		thisNode->G = distanceViaLastNode;
		thisNode->H = thisNode->actualDistance(_endCell);
		thisNode->lastNode = lastNode;
	}
}