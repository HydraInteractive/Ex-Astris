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
	intializedStartGoal = false;
	foundGoal = false;
}

PathFinding::~PathFinding(){

}

void PathFinding::findPath(const glm::vec3& currentPos, const glm::vec3& targetPos, int (&map)[WORLD_SIZE][WORLD_SIZE])
{
	if (!intializedStartGoal) 
	{
		_openList.clear();
		_visitedList.clear();
		_pathToEnd.clear();
		
		_startCell = std::make_shared<Node>(currentPos.x / CELL_SIZE, currentPos.z / CELL_SIZE, nullptr);
		_endCell = std::make_shared<Node>(targetPos.x / CELL_SIZE, targetPos.z / CELL_SIZE, nullptr);

		_startCell->H = _startCell->actualDistance(_endCell);
		_openList.push_back(_startCell);

		foundGoal = false;
		intializedStartGoal = true;
	}
	//TODO: Infinite loop concerns
	while(!_openList.empty() && !foundGoal)
	{
		std::shared_ptr<Node> currentCell = _getNextCell();

		//End reached
		if (currentCell->id == _endCell->id)
		{
			_endCell->parent = currentCell->parent;

			std::shared_ptr<Node> getPath = _endCell;

			while (getPath != nullptr)
			{
				_pathToEnd.push_back(MapVec(getPath->pos.x() * CELL_SIZE, getPath->pos.z() * CELL_SIZE));
				getPath = getPath->parent;
			}
			foundGoal = true;
		}
		//Navigate map
		else
		{
			//East
			_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z(), currentCell->G + 1, currentCell, map);
			
			//West
			_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z(), currentCell->G + 1, currentCell, map);
			
			//North
			_discoverNode(currentCell->pos.x(), currentCell->pos.z() + 1, currentCell->G + 1, currentCell, map);
			
			//South
			_discoverNode(currentCell->pos.x(), currentCell->pos.z() - 1, currentCell->G + 1, currentCell, map);
			
			//North West
			_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z() + 1, currentCell->G + 1.414f, currentCell, map);
			
			//North East
			_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z() + 1, currentCell->G + 1.414f, currentCell, map);
			
			//South West
			_discoverNode(currentCell->pos.x() - 1, currentCell->pos.z() - 1, currentCell->G + 1.414f, currentCell, map);
			
			//South East
			_discoverNode(currentCell->pos.x() + 1, currentCell->pos.z() - 1, currentCell->G + 1.414f, currentCell, map);

		}
		for (size_t i = 0; i < _openList.size(); i++)
		{
			if (currentCell->id == _openList[i]->id)
			{
				_openList.erase(_openList.begin() + i);
			}
		}
	}
}

glm::vec3 PathFinding::nextPathPos(const glm::vec3& pos, const float& radius)
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

void PathFinding::_discoverNode(int x, int z, float newCost, std::shared_ptr<Node> parent, int(&map)[WORLD_SIZE][WORLD_SIZE])
{
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

	std::shared_ptr<Node> newCell = std::make_shared<Node>(x, z, parent);

	newCell->G = newCost;
	newCell->H = parent->actualDistance(_endCell);

	for (size_t i = 0; i < _openList.size(); i++)
	{
		if (id == _openList[i]->id)
		{
			float newF = newCell->G + newCost + _openList[i]->H;
			
			if (_openList[i]->getF() > newF)
			{
				_openList[i]->G = newCell->G + newCost;
				_openList[i]->parent = newCell;
			}
			else // if the F-value is not better
			{
				return;
			}
		}
	}

	_openList.push_back(newCell);
}

std::shared_ptr<PathFinding::Node> PathFinding::_getNextCell()
{
	float bestF = 999999.0f;
	int cellID = -1;
	std::shared_ptr<Node> nextCell;

	for (size_t i = 0; i < _openList.size(); i++)
	{
		if (_openList[i]->getF() < bestF)
		{
			bestF = _openList[i]->getF();
			cellID = i;
		}
	}

	if (cellID >= 0)
	{
		nextCell = _openList[cellID];
		_visitedList.push_back(nextCell);
		_openList.erase(_openList.begin() + cellID);
	}

	return nextCell;
}