/*
 * ActionModelAddCube.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCube.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"

ActionModelAddCube::ActionModelAddCube(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3)
{
	// check orientation
	vector dv1 = _dv1;
	vector dv2 = _dv2;
	vector dv3 = _dv3;
	if ((dv1 ^ dv2) * dv3 < 0){
		dv1 = _dv2;
		dv2 = _dv1;
	}
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	// 8 vertices
	AddSubAction(new ActionModelAddVertex(vector(_pos                  )), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos + dv1            )), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos       + dv2      )), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos + dv1 + dv2      )), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos             + dv3)), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos + dv1       + dv3)), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos       + dv2 + dv3)), m);
	AddSubAction(new ActionModelAddVertex(vector(_pos + dv1 + dv2 + dv3)), m);

	// front
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 0, nv + 2, nv + 3, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 0, nv + 3, nv + 1, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);
	// top
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 2, nv + 6, nv + 7, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 2, nv + 7, nv + 3, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);
	// bottom
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 4, nv + 0, nv + 1, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 4, nv + 1, nv + 5, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);
	// left
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 4, nv + 6, nv + 2, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 4, nv + 2, nv + 0, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);
	// right
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 1, nv + 3, nv + 7, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 1, nv + 7, nv + 5, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);
	// back
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 5, nv + 7, nv + 6, material, vector(0,1,0), vector(0,0,0), vector(1,0,0)), m);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv + 5, nv + 6, nv + 4, material, vector(0,1,0), vector(1,0,0), vector(1,1,0)), m);

}

ActionModelAddCube::~ActionModelAddCube()
{
}


