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

void PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos, int map[WORLD_SIZE][WORLD_SIZE])
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

		// Initialize start
		SearchCell start;
		start.m_xcoord = currentPos.x / CELL_SIZE;
		start.m_zcoord = currentPos.z / CELL_SIZE;

		// Initialize end
		SearchCell end;
		end.m_xcoord = targetPos.x / CELL_SIZE;
		end.m_zcoord = targetPos.z / CELL_SIZE;

		foundGoal = false;
		_setStartAndGoal(start, end);
		intializedStartGoal = true;
	}

	if (intializedStartGoal)
	{
		_continuePath(map);
	}
}

glm::vec3 PathFinding::nextPathPos(glm::vec3 pos, float radius)
{
	size_t index = 1;

	glm::vec3 nextPos;
	nextPos.x = _pathToEnd[_pathToEnd.size() - index].x + (CELL_SIZE / 2);
	nextPos.z = _pathToEnd[_pathToEnd.size() - index].z + (CELL_SIZE / 2);
	
	float distance = glm::distance(pos, nextPos);

	if (index < _pathToEnd.size())
	{
		if (distance < radius)
		{
			_pathToEnd.pop_back();
		}
	}
	return nextPos;
}

void PathFinding::_setStartAndGoal(SearchCell start, SearchCell end)
{
	_startCell = std::make_shared<SearchCell>(start.m_xcoord, start.m_zcoord, nullptr);
	_endCell = std::make_shared<SearchCell>(end.m_xcoord, end.m_zcoord, std::make_shared<SearchCell>(end));

	_startCell->G = 0;
	_startCell->H = _startCell->manHattanDistance(_endCell);
	_startCell->parent = nullptr;
	_openList.push_back(_startCell);
}

void PathFinding::_pathOpened(int x, int z, float newCost, std::shared_ptr<SearchCell> parent, int map[WORLD_SIZE][WORLD_SIZE])
{
	if (map[x][z] == 1 || map[x][z] == 2)
	{
		return;
	}

	int id = z * WORLD_SIZE + x;
	for (size_t i = 0; i < _visitedList.size(); i++)
	{
		if (id == _visitedList[i]->m_id)
		{
			return;
		}
	}

	std::shared_ptr<SearchCell> newCell = std::make_shared<SearchCell>(x, z, parent);

	newCell->G = newCost;
	newCell->H = parent->manHattanDistance(_endCell);

	for (size_t i = 0; i < _openList.size(); i++)
	{
		if (id == _openList[i]->m_id)
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

std::shared_ptr<SearchCell> PathFinding::_getNextCell()
{
	float bestF = 999999.0f;
	int cellID = -1;
	std::shared_ptr<SearchCell> nextCell;

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

void PathFinding::_continuePath(int map[WORLD_SIZE][WORLD_SIZE])
{
	for (size_t i = 0; i < 4; i++)
	{
		if (_openList.empty())
		{
			return;
		}

		std::shared_ptr<SearchCell> currentCell = _getNextCell();

		if (currentCell->m_id == _endCell->m_id)
		{
			_endCell->parent = currentCell->parent;

			std::shared_ptr<SearchCell> getPath;

			for (getPath = _endCell; getPath != NULL; getPath = getPath->parent)
			{
				_pathToEnd.push_back(glm::vec3(getPath->m_xcoord * CELL_SIZE, 0, getPath->m_zcoord * CELL_SIZE));
			}
			foundGoal = true;
		}
		else
		{
			//rightSide
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord, currentCell->G + 1, currentCell, map);
			//leftSide
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord, currentCell->G + 1, currentCell, map);
			//upSide
			_pathOpened(currentCell->m_xcoord, currentCell->m_zcoord + 1, currentCell->G + 1, currentCell, map);
			//downSide
			_pathOpened(currentCell->m_xcoord, currentCell->m_zcoord - 1, currentCell->G + 1, currentCell, map);
			//left-up diagonal
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord + 1, currentCell->G + 1.414f, currentCell, map);
			//right-up diagonal
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord + 1, currentCell->G + 1.414f, currentCell, map);
			//left-down diagonal
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord - 1, currentCell->G + 1.414f, currentCell, map);
			//right-down diagonal
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord - 1, currentCell->G + 1.414f, currentCell, map);

			for (size_t i = 0; i < _openList.size(); i++)
			{
				if (currentCell->m_id == _openList[i]->m_id)
				{
					_openList.erase(_openList.begin() + i);
				}
			}
		}
	}
}

