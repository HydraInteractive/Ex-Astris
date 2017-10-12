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
	struct MapVec
	{
		glm::vec2 baseVec = glm::vec2(1.0f,2.0f);
		
		float& x() { return baseVec.x; }
		float& z() { return baseVec.y; }
		glm::vec3& vec3() { return glm::vec3(baseVec.x, 0, baseVec.y); }
		void set(glm::vec2 &vec) { baseVec = vec; }
		void set(glm::vec3 &vec) { baseVec = glm::vec2(vec.x,vec.z); }
		glm::vec3 MapVec::operator=(glm::vec3 vec) { return glm::vec3(baseVec.x, 0, baseVec.y); }
		glm::vec2 MapVec::operator=(glm::vec2 vec) { return baseVec; }
	};
	struct Node
	{
		//X and Y represent the 2D map's dimensions not the world's. X = worldX, Y = worldZ
		MapVec pos;
		int id;
		std::shared_ptr<Node> parent;
		float G;
		float H;

		Node() : parent() {}
		Node(int x, int z, std::shared_ptr<Node> _parent = nullptr)
		{
			pos.x() = x;
			pos.z() = z;
			id = z * WORLD_SIZE + x;
			parent = _parent;
			G = 0.0f;
			H = 0.0f;
		}

		float getF() { return G + H; }
		float manHattanDistance(std::shared_ptr<Node> nodeEnd)
		{
			float x = (float)(fabs((float)(this->pos.x() - nodeEnd->pos.x())));
			float z = (float)(fabs((float)(this->pos.z() - nodeEnd->pos.z())));

			return x + z;
		}
	};

	PathFinding();
	virtual ~PathFinding();

	void findPath(glm::vec3 currentPos, glm::vec3 targetPos, int(&map)[WORLD_SIZE][WORLD_SIZE]);
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
	std::shared_ptr<Node> _startCell;
	std::shared_ptr<Node> _endCell;

	void _pathOpened(int x, int z, float newCost, std::shared_ptr<Node> parent, int(&map)[WORLD_SIZE][WORLD_SIZE]);
	std::shared_ptr<Node> _getNextCell();
};
