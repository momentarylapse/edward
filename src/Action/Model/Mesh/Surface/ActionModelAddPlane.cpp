/*
 * ActionModelAddPlane.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddPlane.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include <assert.h>

ActionModelAddPlane::ActionModelAddPlane(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, int _num_x, int _num_y)
{
	assert(_num_x * _num_y > 0);

	/// vertices
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;
	vector dx = _dv1 / _num_x;
	vector dy = _dv2 / _num_y;
	for (int x=0;x<_num_x+1;x++)
		for (int y=0;y<_num_y+1;y++)
			AddSubAction(new ActionModelAddVertex(_pos + dx*(float)x + dy*(float)y), m);

	// triangles
	for (int x=0;x<_num_x;x++)
		for (int y=0;y<_num_y;y++){
			vector sva = vector((float) x   /(float)_num_x,(float) y   /(float)_num_y,0);
			vector svb = vector((float)(x+1)/(float)_num_x,(float) y   /(float)_num_y,0);
			vector svc = vector((float) x   /(float)_num_x,(float)(y+1)/(float)_num_y,0);
			vector svd = vector((float)(x+1)/(float)_num_x,(float)(y+1)/(float)_num_y,0);
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+(_num_y+1)* x   +y+1,
					nv+(_num_y+1)* x   +y,
					nv+(_num_y+1)*(x+1)+y,
					material,
			        svc, sva, svb), m);
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+(_num_y+1)* x   +y+1,
					nv+(_num_y+1)*(x+1)+y,
					nv+(_num_y+1)*(x+1)+y+1,
					material,
					svc, svb, svd), m);
		}
}

ActionModelAddPlane::~ActionModelAddPlane()
{
}
