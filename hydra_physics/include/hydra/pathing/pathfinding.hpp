/**
* A* pathfinding
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <hydra/pathing/SearchCell.hpp>

class PathFinding
{
public:
	PathFinding();
	virtual ~PathFinding();

	void findPath(glm::vec3 currentPos, glm::vec3 targetPos, int map[WORLD_SIZE][WORLD_SIZE]);
	glm::vec3 nextPathPos(glm::vec3 pos, float radius);
	void clearOpenList() { _openList.clear(); }
	void clearVisitedList() { _visitedList.clear(); }
	void clearPathToGoal() { _pathToEnd.clear(); }
	bool intializedStartGoal;
	bool foundGoal;
	std::vector<std::shared_ptr<SearchCell>> _openList;
	std::vector<std::shared_ptr<SearchCell>> _visitedList;
	std::vector<glm::vec3> _pathToEnd;
private:
	void _setStartAndGoal(SearchCell start, SearchCell end);
	void _pathOpened(int x, int z, float newCost, std::shared_ptr<SearchCell> parent, int map[WORLD_SIZE][WORLD_SIZE]);
	std::shared_ptr<SearchCell> _getNextCell();
	void _continuePath(int map[WORLD_SIZE][WORLD_SIZE]);

	std::shared_ptr<SearchCell> _startCell;
	std::shared_ptr<SearchCell> _endCell;

};
