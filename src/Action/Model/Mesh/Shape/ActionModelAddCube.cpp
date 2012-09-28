/*
 * ActionModelAddCube.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCube.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"

static Array<int> quad(int a, int b, int c, int d)
{
	Array<int> v;
	v.add(a);
	v.add(b);
	v.add(c);
	v.add(d);
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
	v = quad(nv + 1, nv + 0, nv + 2, nv + 3);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	// top
	v = quad(nv + 3, nv + 2, nv + 6, nv + 7);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	// bottom
	v = quad(nv + 5, nv + 4, nv + 0, nv + 1);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	// left
	v = quad(nv + 0, nv + 4, nv + 6, nv + 2);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	// right
	v = quad(nv + 5, nv + 1, nv + 3, nv + 7);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	// back*/
	v = quad(nv + 4, nv + 5, nv + 7, nv + 6);
	AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);

}

ActionModelAddCube::~ActionModelAddCube()
{
}

void *ActionModelAddCube::compose(Data *d)
{	return &(dynamic_cast<DataModel*>(d))->Surface.back();	}


