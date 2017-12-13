/**
* A* pathfinding
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <math.h>
#include <algorithm>
#include <hydra/component/roomcomponent.hpp>

class HYDRA_PHYSICS_API PathFinding
{
public:
	//A 2D vector with x and z axies instead of x and y
	struct MapVec
	{
		glm::ivec2 baseVec;

		MapVec(int x, int z) { baseVec = glm::vec2(x, z); }
		MapVec() { baseVec = glm::vec2(0.0f, 0.0f); }

		int& x() { return baseVec.x; }
		int& z() { return baseVec.y; }
		void set(const glm::vec2 &vec) { baseVec = vec; }
		void set(const glm::vec3 &vec) { baseVec = glm::vec2(vec.x, vec.z); }

		bool operator==(const MapVec& other) { return this->baseVec == other.baseVec; }
		//operator glm::vec3() { return glm::ivec3(baseVec.x, 0, baseVec.y); }
		operator glm::vec2() const { return baseVec; }
	};
	struct Node
	{
		MapVec pos = MapVec();
		Node* lastNode = nullptr;
		float G = 0.0f;
		float H = 0.0f;
		Node() {}
		Node(int x, int z, Node* lastNode = nullptr)
		{
			this->pos.x() = x;
			this->pos.z() = z;
			this->lastNode = lastNode;
		}

		float getF() const { return G + H; }

		//Manhattan Distance - do not use, gives invalid values
		//Distance to side nodes is 1, diagonal is 2
		int hDistanceTo(Node* nodeEnd)
		{
			return abs(this->pos.x() - nodeEnd->pos.x() + abs(this->pos.z() - nodeEnd->pos.z()));
		}

		//Chebychev Distance - inaccurate but safe
		//Distance to all adjacent nodes is 1
		//int hDistanceTo(Node* nodeEnd)
		//{
		//	return std::max(abs(this->pos.x() - nodeEnd->pos.x(), abs(this->pos.z() - nodeEnd->pos.z());
		//}

		//Actual Distance - probably the best maybe, float inaccuracies may break it
		//float hDistanceTo(Node* nodeEnd)
		//{
		//	return std::sqrt(std::pow(this->pos.x() - nodeEnd->pos.x(), 2.0f) + std::pow(this->pos.z() - nodeEnd->pos.z(), 2.0f));
		//}

		//Must always be used to calculate G distance
		float gDistanceTo(Node* nodeEnd)
		{
			return std::sqrt(std::pow(this->pos.x() - nodeEnd->pos.x(), 2.0f) + std::pow(this->pos.z() - nodeEnd->pos.z(), 2.0f));
		}
		bool operator<(Node& other) { return this->getF() < other.getF(); }
		bool operator==(Node& other) { return this->getF() == other.getF(); }
		bool operator>(Node& other) { return this->getF() > other.getF(); }
	};

	bool foundGoal = false;
	std::vector<glm::vec3> pathToEnd = std::vector<glm::vec3>();
	bool** map = nullptr;

	PathFinding();
	virtual ~PathFinding();

	bool findPath(glm::vec3 currentPos, glm::vec3 targetPos);
	static MapVec worldToMapCoords(const glm::vec3& worldPos);
	static glm::vec3 mapToWorldCoords(const MapVec& mapPos);
	bool inLineOfSight(const glm::vec3& enemyPos, const glm::vec3& targetPos) const;
	bool inWall(const glm::vec3 mapPos) const;
	glm::vec3 findViableTile(glm::vec3 mapPos) const;

	struct {
		bool operator()(const Node* _Left, const Node* _Right) const
		{
			if (_Left == nullptr)
			{
				return false;
			}
			else if (_Right == nullptr)
			{
				return true;
			}
			return (_Left->getF() > _Right->getF());
		}
	} comparisonFunctor;

private:
	std::vector<Node*> _visitedList = std::vector<Node*>();
	std::vector<Node*> _openList = std::vector<Node*>();
	Node* _startNode = nullptr;
	Node* _endNode = nullptr;

	bool isOutOfBounds(const glm::ivec2& vec) const;
	bool isOutsideMap(const glm::ivec2& vec) const;
	bool _inLineOfSight(const MapVec enemyPos, const MapVec playerPos) const;
	void _discoverNode(int x, int z, Node* lastNode);
};