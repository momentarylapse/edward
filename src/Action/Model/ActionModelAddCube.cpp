/*
 * ActionModelAddCube.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCube.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangle.h"

ActionModelAddCube::ActionModelAddCube(const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int nv)
{
	// check orientation
	vector dv1 = _dv1;
	vector dv2 = _dv2;
	vector dv3 = _dv3;
	if ((dv1 ^ dv2) * dv3 < 0){
		dv1 = _dv2;
		dv2 = _dv1;
	}

	// 8 vertices
	action.add(new ActionModelAddVertex(vector(_pos                  )));
	action.add(new ActionModelAddVertex(vector(_pos + dv1            )));
	action.add(new ActionModelAddVertex(vector(_pos       + dv2      )));
	action.add(new ActionModelAddVertex(vector(_pos + dv1 + dv2      )));
	action.add(new ActionModelAddVertex(vector(_pos             + dv3)));
	action.add(new ActionModelAddVertex(vector(_pos + dv1       + dv3)));
	action.add(new ActionModelAddVertex(vector(_pos       + dv2 + dv3)));
	action.add(new ActionModelAddVertex(vector(_pos + dv1 + dv2 + dv3)));

	// front
	action.add(new ActionModelAddTriangle(nv + 0, nv + 2, nv + 3, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 0, nv + 3, nv + 1, vector(0,1,0), vector(1,0,0), vector(1,1,0)));
	// top
	action.add(new ActionModelAddTriangle(nv + 2, nv + 6, nv + 7, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 2, nv + 7, nv + 3, vector(0,1,0), vector(1,0,0), vector(1,1,0)));
	// bottom
	action.add(new ActionModelAddTriangle(nv + 4, nv + 0, nv + 1, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 4, nv + 1, nv + 5, vector(0,1,0), vector(1,0,0), vector(1,1,0)));
	// left
	action.add(new ActionModelAddTriangle(nv + 4, nv + 6, nv + 2, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 4, nv + 2, nv + 0, vector(0,1,0), vector(1,0,0), vector(1,1,0)));
	// right
	action.add(new ActionModelAddTriangle(nv + 1, nv + 3, nv + 7, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 1, nv + 7, nv + 5, vector(0,1,0), vector(1,0,0), vector(1,1,0)));
	// back
	action.add(new ActionModelAddTriangle(nv + 5, nv + 7, nv + 6, vector(0,1,0), vector(0,0,0), vector(1,0,0)));
	action.add(new ActionModelAddTriangle(nv + 5, nv + 6, nv + 4, vector(0,1,0), vector(1,0,0), vector(1,1,0)));

}

ActionModelAddCube::~ActionModelAddCube()
{
}


