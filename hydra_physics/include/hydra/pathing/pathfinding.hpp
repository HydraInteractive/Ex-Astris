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
#include <hydra/pathing/SearchCell.hpp>
#include <hydra/component/aicomponent.hpp>

class PathFinding
{
public:
	PathFinding(void);
	~PathFinding(void);

	void findPath(glm::vec3 currentPos, glm::vec3 targetPos);
	glm::vec3 nextPathPos(Hydra::Component::EnemyComponent ai);
	void clearOpenList() { _m_openList.clear(); }
	void clearVisitedList() { _m_visitedList.clear(); }
	void clearPathToGoal() { _m_pathToEnd.clear(); }
	bool m_intializedStartGoal;
	bool m_foundGoal;
private:
	void _setStartAndGoal(SearchCell start, SearchCell end);
	void _pathOpened(int x, int z, float newCost, SearchCell *parent);
	SearchCell *_getNextCell();
	void _continuePath();

	SearchCell *_m_startCell;
	SearchCell *_m_endCell;
	std::vector<SearchCell*> _m_openList;
	std::vector<SearchCell*> _m_visitedList;
	std::vector<glm::vec3*> _m_pathToEnd;
};
