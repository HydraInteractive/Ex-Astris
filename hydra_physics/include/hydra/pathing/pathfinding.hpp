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
#define WORLD_SIZE 64
#define CELL_SIZE 1.0f
#include <math.h>


class PathFinding
{
public:
	class Node
	{
	public:
		int m_xcoord, m_zcoord;
		int m_id;
		std::shared_ptr<Node> parent;
		float G;
		float H;

		Node() : parent() {}
		Node(int x, int z, std::shared_ptr<Node> _parent) : m_xcoord(x), m_zcoord(z),
			m_id(z * WORLD_SIZE + x), parent(_parent), G(0), H(0) {}

		float getF() { return G + H; }
		float manHattanDistance(std::shared_ptr<Node> nodeEnd)
		{
			float x = (float)(fabs((float)(this->m_xcoord - nodeEnd->m_xcoord)));
			float z = (float)(fabs((float)(this->m_zcoord - nodeEnd->m_zcoord)));

			return x + z;
		}
	};

	PathFinding();
	virtual ~PathFinding();

	void findPath(glm::vec3 currentPos, glm::vec3 targetPos, int map[WORLD_SIZE][WORLD_SIZE]);
	glm::vec3 nextPathPos(glm::vec3 pos, float radius);
	void clearOpenList() { _openList.clear(); }
	void clearVisitedList() { _visitedList.clear(); }
	void clearPathToGoal() { _pathToEnd.clear(); }
	bool intializedStartGoal;
	bool foundGoal;
	std::vector<std::shared_ptr<Node>> _openList;
	std::vector<std::shared_ptr<Node>> _visitedList;
	std::vector<glm::vec3> _pathToEnd;
	
private:

	void _setStartAndGoal(Node start, Node end);
	void _pathOpened(int x, int z, float newCost, std::shared_ptr<Node> parent, int map[WORLD_SIZE][WORLD_SIZE]);
	std::shared_ptr<Node> _getNextCell();
	void _continuePath(int map[WORLD_SIZE][WORLD_SIZE]);
	std::shared_ptr<Node> _startCell;
	std::shared_ptr<Node> _endCell;
};
