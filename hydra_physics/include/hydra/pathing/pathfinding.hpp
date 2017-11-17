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
		operator glm::vec2() { return baseVec; }
	};
	struct Node
	{
		MapVec pos;
		std::shared_ptr<Node> lastNode;
		float G = 0.0f;
		float H = 0.0f;
		float F = 0.0f;
		Node() {}
		Node(int x, int z, std::shared_ptr<Node> lastNode = nullptr)
		{
			this->pos.x() = x;
			this->pos.z() = z;
			this->lastNode = lastNode;
		}

		float getF() { F = G + H; return F; }

		//Manhattan Distance - do not use, gives invalid values
		//Distance to adjacent nodes is 1, diagonal is 2
		int hDistanceTo(std::shared_ptr<Node> nodeEnd)
		{
			int x = abs(this->pos.x() - nodeEnd->pos.x());
			int z = abs(this->pos.z() - nodeEnd->pos.z());
			return x + z;
		}

		//Chebychev Distance - inaccurate but safe
		//Distance to adjacent nodes is 1
		//int hDistanceTo(std::shared_ptr<Node> nodeEnd)
		//{
		//	int x = abs(this->pos.x() - nodeEnd->pos.x());
		//	int z = abs(this->pos.z() - nodeEnd->pos.z());
		//	return std::max(x, z);
		//}

		//Actual Distance - probably the best maybe, float inaccuracies may break it
		//float hDistanceTo(std::shared_ptr<Node> nodeEnd)
		//{
		//	return std::sqrt(std::pow(this->pos.x() - nodeEnd->pos.x(), 2.0f) + std::pow(this->pos.z() - nodeEnd->pos.z(), 2.0f) * 0.99);
		//}

		//Must always be used to calculate G distance
		float gDistanceTo(std::shared_ptr<Node> nodeEnd)
		{
			return std::sqrt(std::pow(this->pos.x() - nodeEnd->pos.x(), 2.0f) + std::pow(this->pos.z() - nodeEnd->pos.z(), 2.0f));
		}
		bool operator<(Node& other) { return this->getF() < other.getF(); }
		bool operator==(Node& other) { return this->getF() == other.getF(); }
		bool operator>(Node& other) { return this->getF() > other.getF(); }
	};

	PathFinding();
	virtual ~PathFinding();

	bool intializedStartGoal;
	bool foundGoal;
	std::vector<glm::vec3> _pathToEnd;
	bool** map = nullptr;

	void findPath(const glm::vec3& currentPos, const glm::vec3& targetPos);
	glm::vec3 nextPathPos(const glm::vec3& pos, const float& radius);
	MapVec worldToMapCoords(const glm::vec3& worldPos) const;
	glm::vec3 mapToWorldCoords(const MapVec& mapPos) const;

	bool intializedStartGoal;
	bool foundGoal;
	float enemyY = 0.0f;

	struct {
		bool operator()(const std::shared_ptr<Node>& _Left, const std::shared_ptr<Node>& _Right) const
		{
			if (_Left == nullptr)
			{
				return 0;
			}
			else if (_Right == nullptr)
			{
				return 1;
			}
			return (_Left->getF() > _Right->getF());
		}
	} comparisonFunctor;
	
private:
	std::vector<std::shared_ptr<Node>> _visitedList;
	std::vector<std::shared_ptr<Node>> _openList;
	std::shared_ptr<Node> _startNode;
	std::shared_ptr<Node> _endNode;

	bool isOutOfBounds(const glm::vec2& vec)const;
	void _discoverNode(int x, int z, std::shared_ptr<Node> lastNode);
};
