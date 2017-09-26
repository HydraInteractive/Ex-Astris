/**
* Used for pathfinding
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#define WORLD_SIZE 64
#define CELL_SIZE 10.0f
#include <math.h>

struct SearchCell
{
	public:
		int m_xcoord, m_zcoord;
		int m_id;
		SearchCell *parent;
		float G;
		float H;

		SearchCell() : parent(0) {}
		SearchCell(int x, int z, SearchCell *_parent = 0) : m_xcoord(x), m_zcoord(z),
			parent(_parent), m_id(z * WORLD_SIZE + x), G(0), H(0) {}

		float getF() { return G + H; }
		float manHattanDistance(SearchCell *nodeEnd)
		{
			float x = (float)(fabs((float)(this->m_xcoord - nodeEnd->m_xcoord)));
			float z = (float)(fabs((float)(this->m_zcoord - nodeEnd->m_zcoord)));

			return x + z;
		}
};
