/*
 * ActionModelAddTorus.cpp
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#include "ActionModelAddTorus.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddTorus::ActionModelAddTorus(const vector &_pos, const vector &_axis, float _radius1, float _radius2, int _num_x, int _num_y) :
	pos(_pos), axis(_axis)
{
	radius1 = fabs(_radius1);
	radius2 = fabs(_radius2);
	num_x = _num_x;
	num_y = _num_y;
}

#define _tor_vert(i, j)         ( num_y * ((i) % num_x) + ((j) % num_y)) + nv

void *ActionModelAddTorus::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	vector d1 = axis.ortho();
	vector d2 = d1 ^ axis;

	int nv = m->Vertex.num;

	// create vertices
	for (int i=0;i<num_x;i++)
		for (int j=0;j<num_y;j++){
			float wx = (float)i / (float)num_x * 2 * pi;
			float wy = (float)j / (float)num_y * 2 * pi;
			vector e_r = d1 * cos(wx) + d2 * sin(wx);
			vector p = pos + e_r * radius1 + (e_r * cos(wy) + axis * sin(wy)) * radius2;
			AddSubAction(new ActionModelAddVertex(p), m);
		}

	// polygons
	for (int i=0;i<num_x;i++)
		for (int j=0;j<num_y;j++){
			Array<int> v;
			v.add(_tor_vert(i+1, j));
			v.add(_tor_vert(i,   j));
			v.add(_tor_vert(i,   j+1));
			v.add(_tor_vert(i+1, j+1));
			Array<vector> sv;
			sv.add(vector((float)(i+1) / (float)num_x, (float) j    / (float)num_y, 0));
			sv.add(vector((float) i    / (float)num_x, (float) j    / (float)num_y, 0));
			sv.add(vector((float) i    / (float)num_x, (float)(j+1) / (float)num_y, 0));
			sv.add(vector((float)(i+1) / (float)num_x, (float)(j+1) / (float)num_y, 0));
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, m->CurrentMaterial, sv), m);
		}

	return NULL;
}

