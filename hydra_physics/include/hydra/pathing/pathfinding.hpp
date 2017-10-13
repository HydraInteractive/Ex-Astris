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
#include <queue>
#include <functional>

class PathFinding
{
public:
	//A 2D vector with x and z axies instead of x and y
	struct MapVec
	{
		glm::vec2 baseVec;

		MapVec(float x, float z) { baseVec = glm::vec2(x, z); }
		MapVec() { baseVec = glm::vec2(0.0f, 0.0f); }

		float& x() { return baseVec.x; }
		float& z() { return baseVec.y; }
		glm::vec3& vec3() { return glm::vec3(baseVec.x, 0, baseVec.y); }
		void set(glm::vec2 &vec) { baseVec = vec; }
		void set(glm::vec3 &vec) { baseVec = glm::vec2(vec.x,vec.z); }

		operator glm::vec3()& { return glm::vec3(baseVec.x, 0, baseVec.y); }
		operator glm::vec2()& { return baseVec; }
		
	};
	struct Node
	{
		MapVec pos;
		int id;
		std::shared_ptr<Node> lastNode;
		float G = 0.0f;
		float H = 0.0f;

		Node() {}
		Node(int x, int z, std::shared_ptr<Node> lastNode = nullptr)
		{
			this->pos.x() = x;
			this->pos.z() = z;
			this->id = z * WORLD_SIZE + x;
			this->lastNode = lastNode;
		}

		float getF() { return G + H; }
		//float manHattanDistance(std::shared_ptr<Node> nodeEnd)
		//{
		//	float x = fabs((float)(this->pos.x() - nodeEnd->pos.x()));
		//	float z = fabs((float)(this->pos.z() - nodeEnd->pos.z()));
		//	return x + z;
		//}
		//float chebyshevDistance(std::shared_ptr<Node> nodeEnd)
		//{
		//	float x = fabs((float)(this->pos.x() - nodeEnd->pos.x()));
		//	float z = fabs((float)(this->pos.z() - nodeEnd->pos.z()));
		//	return std::fmax(x, z);
		//}
		float actualDistance(std::shared_ptr<Node> nodeEnd)
		{
			return std::sqrt(std::pow(this->pos.x() - nodeEnd->pos.x(), 2.0f) + std::pow(this->pos.z() - nodeEnd->pos.z(), 2.0f));
		}
		bool operator<(Node& other) { return this->getF() < other.getF(); }
		bool operator==(Node& other) { return this->getF() == other.getF(); }
		bool operator>(Node& other) { return this->getF() > other.getF(); }
	};

	PathFinding();
	virtual ~PathFinding();

	void findPath(const glm::vec3& currentPos, const glm::vec3& targetPos, int(&map)[WORLD_SIZE][WORLD_SIZE]);
	glm::vec3& nextPathPos(const glm::vec3& pos, const float& radius);
	void clearVisitedList() { _visitedList.clear(); }
	void clearOpenList()
	{ 
		_openList = PriorityQueue();
	}
	void clearPathToGoal() { _pathToEnd.clear(); }

	bool intializedStartGoal;
	bool foundGoal;
	struct QueueCompare {
		bool operator ()(std::shared_ptr<Node> left, std::shared_ptr<Node> right) { return left->getF() < right->getF(); }
	};
	typedef std::priority_queue < std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, QueueCompare> PriorityQueue;

	std::vector<std::shared_ptr<Node>> _visitedList;
	std::vector<MapVec> _pathToEnd;
	
private:
	PriorityQueue _openList;
	std::shared_ptr<Node> _startCell;
	std::shared_ptr<Node> _endCell;

	void _discoverNode(int x, int z, std::shared_ptr<Node> lastNode, int(&map)[WORLD_SIZE][WORLD_SIZE]);
};
