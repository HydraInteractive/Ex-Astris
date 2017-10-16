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

void PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos, int (&map)[WORLD_SIZE][WORLD_SIZE])
{
	if (!intializedStartGoal) 
	{

		for (size_t i = 0; i < _openList.size(); i++)
		{
			//delete _openList[i];
		}
		_openList.clear();

		for (size_t i = 0; i < _visitedList.size(); i++) 
		{
			//delete _visitedList[i];
		}
		_visitedList.clear();

		for (size_t i = 0; i < _pathToEnd.size(); i++)
		{
			//delete _pathToEnd[i];
		}
		_pathToEnd.clear();
		
		_startCell = std::make_shared<Node>(currentPos.x / CELL_SIZE, currentPos.z / CELL_SIZE, nullptr);
		_endCell = std::make_shared<Node>(targetPos.x / CELL_SIZE, targetPos.z / CELL_SIZE, nullptr);

		_startCell->H = _startCell->chebyshevDistance(_endCell);
		_openList.push_back(_startCell);

		foundGoal = false;
		intializedStartGoal = true;
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (_openList.empty())
		{
			return;
		}

		std::shared_ptr<Node> currentCell = _getNextCell();

		if (currentCell->id == _endCell->id)
		{
			_endCell->parent = currentCell->parent;

			std::shared_ptr<Node> getPath;

			for (getPath = _endCell; getPath != NULL; getPath = getPath->parent)
			{
				_pathToEnd.push_back(glm::vec3(getPath->pos.x() * CELL_SIZE, 0, getPath->pos.z() * CELL_SIZE));
			}
			foundGoal = true;
		}
		else
		{
			//East
			_pathOpened(currentCell->pos.x() + 1, currentCell->pos.z(), currentCell->G + 1, currentCell, map);
			
			//West
			_pathOpened(currentCell->pos.x() - 1, currentCell->pos.z(), currentCell->G + 1, currentCell, map);
			
			//North
			_pathOpened(currentCell->pos.x(), currentCell->pos.z() + 1, currentCell->G + 1, currentCell, map);
			
			//South
			_pathOpened(currentCell->pos.x(), currentCell->pos.z() - 1, currentCell->G + 1, currentCell, map);
			
			//North West
			_pathOpened(currentCell->pos.x() - 1, currentCell->pos.z() + 1, currentCell->G + 1.414f, currentCell, map);
			
			//North East
			_pathOpened(currentCell->pos.x() + 1, currentCell->pos.z() + 1, currentCell->G + 1.414f, currentCell, map);
			
			//South West
			_pathOpened(currentCell->pos.x() - 1, currentCell->pos.z() - 1, currentCell->G + 1.414f, currentCell, map);
			
			//South East
			_pathOpened(currentCell->pos.x() + 1, currentCell->pos.z() - 1, currentCell->G + 1.414f, currentCell, map);

			for (size_t i = 0; i < _openList.size(); i++)
			{
				if (currentCell->id == _openList[i]->id)
				{
					_openList.erase(_openList.begin() + i);
				}
			}
		}
	}
}

glm::vec3 PathFinding::nextPathPos(glm::vec3 pos, float radius)
{
	glm::vec3 nextPos;
	nextPos.x = _pathToEnd.back().x + (CELL_SIZE / 2);
	nextPos.z = _pathToEnd.back().z + (CELL_SIZE / 2);
	
	float distance = glm::distance(pos, nextPos);

	if (1 < _pathToEnd.size())
	{
		if (distance < radius)
		{
			_pathToEnd.pop_back();
		}
	}
	return nextPos;
}

void PathFinding::_pathOpened(int x, int z, float newCost, std::shared_ptr<Node> parent, int(&map)[WORLD_SIZE][WORLD_SIZE])
{
	if (map[x][z] == 1 || map[x][z] == 2)
	{
		return;
	}

	int id = z * WORLD_SIZE + x;
	for (size_t i = 0; i < _visitedList.size(); i++)
	{
		if (id == _visitedList[i]->id)
		{
			return;
		}
	}

	std::shared_ptr<Node> newCell = std::make_shared<Node>(x, z, parent);

	newCell->G = newCost;
	newCell->H = parent->manHattanDistance(_endCell);

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
				//delete newCell;
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

