// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* A* pathfinding
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include "hydra\pathing\pathfinding.hpp"

PathFinding::PathFinding(void){
	m_intializedStartGoal = false;
	m_foundGoal = false;
}

PathFinding::~PathFinding(void){

}

void PathFinding::findPath(glm::vec3 currentPos, glm::vec3 targetPos)
{
	if (!m_intializedStartGoal) 
	{

		for (int i = 0; i < _m_openList.size(); i++)
		{
			delete _m_openList[i];
		}
		_m_openList.clear();

		for (int i = 0; i < _m_visitedList.size(); i++) 
		{
			delete _m_visitedList[i];
		}
		_m_visitedList.clear();

		for (int i = 0; i < _m_pathToEnd.size(); i++)
		{
			delete _m_pathToEnd[i];
		}
		_m_pathToEnd.clear();

		// Initialize start
		SearchCell start;
		start.m_xcoord = currentPos.x;
		start.m_zcoord = currentPos.z;

		// Initialize end
		SearchCell end;
		end.m_xcoord = targetPos.x;
		end.m_zcoord = targetPos.z;

		_setStartAndGoal(start, end);
		m_intializedStartGoal = true;
	}

	if (m_intializedStartGoal)
	{
		_continuePath();
	}
}

glm::vec3 PathFinding::nextPathPos(Hydra::Component::EnemyComponent ai)
{
	int index = 1;

	glm::vec3 nextPos;
	nextPos.x = _m_pathToEnd[_m_pathToEnd.size() - index]->x;
	nextPos.z = _m_pathToEnd[_m_pathToEnd.size() - index]->z;

	glm::vec3 distance = nextPos - ai.getPosition();

	if (index < _m_pathToEnd.size())
	{
		if (distance.length() < ai.getRadius())
		{
			_m_pathToEnd.erase(_m_pathToEnd.end() - index);
		}
	}

	return nextPos;
}

void PathFinding::_setStartAndGoal(SearchCell start, SearchCell end)
{
	_m_startCell = new SearchCell(start.m_xcoord, start.m_zcoord, 0);
	_m_endCell = new SearchCell(end.m_xcoord, end.m_zcoord, &end);

	_m_startCell->G = 0;
	_m_startCell->H = _m_startCell->manHattanDistance(_m_endCell);
	_m_startCell->parent = 0;

	_m_openList.push_back(_m_startCell);
}

void PathFinding::_pathOpened(int x, int z, float newCost, SearchCell * parent)
{
	/*if (x * z = wall)
	{
		return;
	}*/

	int id = z * WORLD_SIZE + x;
	for (int i = 0; i < _m_visitedList.size(); i++)
	{
		if (id = _m_visitedList[i]->m_id)
		{
			return;
		}
	}

	SearchCell* newCell = new SearchCell(x, z, parent);

	newCell->G = newCost;
	newCell->H = parent->manHattanDistance(_m_endCell);

	for (int i = 0; i < _m_openList.size(); i++)
	{
		if (id == _m_openList[i]->m_id)
		{
			float newF = newCell->G + newCost + _m_openList[i]->H;
			
			if (_m_openList[i]->getF() > newF)
			{
				_m_openList[i]->G = newCell->G + newCost;
				_m_openList[i]->parent = newCell;
			}
			else // if the F-value is not better
			{
				delete newCell;
				return;
			}
		}
	}

	_m_openList.push_back(newCell);
}

SearchCell * PathFinding::_getNextCell()
{
	float bestF = 999999.0f;
	int cellID = -1;
	SearchCell* nextCell = NULL;

	for (int i = 0; i < _m_openList.size(); i++)
	{
		if (_m_openList[i]->getF() < bestF)
		{
			bestF = _m_openList[i]->getF();
		}
	}

	if (cellID >= 0)
	{
		nextCell = _m_openList[cellID];
		_m_visitedList.push_back(nextCell);
		_m_openList.erase(_m_openList.begin() + cellID);
	}

	return nextCell;
}

void PathFinding::_continuePath()
{
	for (int i = 0; i < 4; i++)
	{
		if (_m_openList.empty())
		{
			return;
		}

		SearchCell* currentCell = _getNextCell();

		if (currentCell->m_id == _m_endCell->m_id)
		{
			_m_endCell->parent = currentCell->parent;

			SearchCell* getPath;

			for (getPath = _m_endCell; getPath != NULL; getPath = getPath->parent)
			{
				_m_pathToEnd.push_back(new glm::vec3(getPath->m_xcoord, 0, getPath->m_zcoord));
			}
			m_foundGoal = true;
			return;
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

			for (int i = 0; i < _m_openList.size(); i++)
			{
				if (currentCell->m_id == _m_openList[i]->m_id)
				{
					_m_openList.erase(_m_openList.begin() + i);
				}
			}
		}
	}
}

