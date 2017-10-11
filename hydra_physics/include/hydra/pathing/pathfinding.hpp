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
		glm::ivec2 pos;
		int m_id;
		std::shared_ptr<Node> parent;
		float G;
		float H;

		Node() : parent() {}
		Node(int x, int z, std::shared_ptr<Node> _parent = nullptr)
		{
			pos.x = x; 
			pos.y = z;
			m_id = z * WORLD_SIZE + x;
			parent = _parent;
			G = 0.0f;
			H = 0.0f;
		}

		float getF() { return G + H; }
		float manHattanDistance(std::shared_ptr<Node> nodeEnd)
		{
			float x = (float)(fabs((float)(this->pos.x - nodeEnd->pos.x)));
			float z = (float)(fabs((float)(this->pos.y - nodeEnd->pos.y)));

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
