/*
 * ActionModelAddCube.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCube.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"

Array<int> quad(int a, int b, int c, int d, int offset)
{
	Array<int> v;
	v.add(a + offset);
	v.add(b + offset);
	v.add(c + offset);
	v.add(d + offset);
	return v;
}

ActionModelAddCube::ActionModelAddCube(DataModel *m, const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int num_1, int num_2, int num_3)
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

	Array<int> v;
	Array<vector> sv;
	sv.add(vector(1,1,0));
	sv.add(vector(0,1,0));
	sv.add(vector(0,0,0));
	sv.add(vector(1,0,0));

	// front
	v = quad(1, 0, 2, 3, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);
	// top
	v = quad(3, 2, 6, 7, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);
	// bottom
	v = quad(5, 4, 0, 1, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);
	// left
	v = quad(0, 4, 6, 2, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);
	// right
	v = quad(5, 1, 3, 7, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);
	// back*/
	v = quad(4, 5, 7, 6, nv);
	AddSubAction(new ActionModelAddTriangleSingleTexture(m, v, material, sv), m);

}

ActionModelAddCube::~ActionModelAddCube()
{
}

void *ActionModelAddCube::compose(Data *d)
{	return &(dynamic_cast<DataModel*>(d))->Surface.back();	}


