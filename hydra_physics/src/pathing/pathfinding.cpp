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

	for (int i = 0; i < WORLD_SIZE; i++)
	{
		for (int j = 0; j < WORLD_SIZE; j++)
		{
			theMap[i][j] = 0;
		}
	}

	for (int i = 0; i < 50; i++)
	{
		theMap[20][10 + i] = 1;
	}
}

PathFinding::~PathFinding(){

}

void PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos)
{
	if (!intializedStartGoal) 
	{

		for (size_t i = 0; i < _openList.size(); i++)
		{
			delete _openList[i];
		}
		_openList.clear();

		for (size_t i = 0; i < _visitedList.size(); i++) 
		{
			delete _visitedList[i];
		}
		_visitedList.clear();

		for (size_t i = 0; i < _pathToEnd.size(); i++)
		{
			delete _pathToEnd[i];
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

		_setStartAndGoal(start, end);
		intializedStartGoal = true;
	}

	if (intializedStartGoal)
	{
		_continuePath();
	}
}

glm::vec3 PathFinding::nextPathPos(glm::vec3 pos, int radius)
{
	int index = 1;

	glm::vec3 nextPos;
	nextPos.x = _pathToEnd[_pathToEnd.size() - index]->x + (CELL_SIZE/2);
	nextPos.z = _pathToEnd[_pathToEnd.size() - index]->z + (CELL_SIZE/2);

	glm::vec3 distance = nextPos - pos;

	if (index < _pathToEnd.size())
	{
		if (distance.length() < radius)
		{
			_pathToEnd.erase(_pathToEnd.end() - index);
		}
	}

	return nextPos;
}

void PathFinding::_setStartAndGoal(SearchCell start, SearchCell end)
{
	_startCell = new SearchCell(start.m_xcoord, start.m_zcoord, 0);
	_endCell = new SearchCell(end.m_xcoord, end.m_zcoord, &end);

	_startCell->G = 0;
	_startCell->H = _startCell->manHattanDistance(_endCell);
	_startCell->parent = 0;

	_openList.push_back(_startCell);
}

void PathFinding::_pathOpened(int x, int z, float newCost, SearchCell * parent)
{
	if (x > WORLD_SIZE * CELL_SIZE || z > WORLD_SIZE * CELL_SIZE || x < (WORLD_SIZE * -1) * CELL_SIZE || z < (WORLD_SIZE * -1) * CELL_SIZE)
	{
		return;
	}

	if (theMap[x][z] == 1)
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

	SearchCell* newCell = new SearchCell(x, z, parent);

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
				delete newCell;
				return;
			}
		}
	}

	_openList.push_back(newCell);
}

SearchCell * PathFinding::_getNextCell()
{
	float bestF = 999999.0f;
	int cellID = -1;
	SearchCell* nextCell = NULL;

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

void PathFinding::_continuePath()
{
	for (size_t i = 0; i < 4; i++)
	{
		if (_openList.empty())
		{
			return;
		}

		SearchCell* currentCell = _getNextCell();

		if (currentCell->m_id == _endCell->m_id)
		{
			_endCell->parent = currentCell->parent;

			SearchCell* getPath;

			for (getPath = _endCell; getPath != NULL; getPath = getPath->parent)
			{
				_pathToEnd.push_back(new glm::vec3(getPath->m_xcoord * CELL_SIZE, 0, getPath->m_zcoord * CELL_SIZE));
			}
			foundGoal = true;
		}
		else
		{
			//rightSide
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord, currentCell->G + 1, currentCell);
			//leftSide
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord, currentCell->G + 1, currentCell);
			//upSide
			_pathOpened(currentCell->m_xcoord, currentCell->m_zcoord + 1, currentCell->G + 1, currentCell);
			//downSide
			_pathOpened(currentCell->m_xcoord, currentCell->m_zcoord - 1, currentCell->G + 1, currentCell);
			//left-up diagonal
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord + 1, currentCell->G + 1.414f, currentCell);
			//right-up diagonal
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord + 1, currentCell->G + 1.414f, currentCell);
			//left-down diagonal
			_pathOpened(currentCell->m_xcoord - 1, currentCell->m_zcoord - 1, currentCell->G + 1.414f, currentCell);
			//right-down diagonal
			_pathOpened(currentCell->m_xcoord + 1, currentCell->m_zcoord - 1, currentCell->G + 1.414f, currentCell);

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

