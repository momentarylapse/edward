/*
 * ActionModelAddPlane.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddPlane.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddPlane::ActionModelAddPlane(const vector &_pos, const vector &_dv1, const vector &_dv2, int _num_x, int _num_y)
{
	pos = _pos;
	dv1 = _dv1;
	dv2 = _dv2;
	num_x = _num_x;
	num_y = _num_y;
}

ActionModelAddPlane::~ActionModelAddPlane()
{
}

void *ActionModelAddPlane::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(num_x * num_y > 0);

	/// vertices
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;
	vector dx = dv1 / num_x;
	vector dy = dv2 / num_y;
	for (int x=0;x<num_x+1;x++)
		for (int y=0;y<num_y+1;y++)
			AddSubAction(new ActionModelAddVertex(pos + dx*(float)x + dy*(float)y), m);

	// triangles
	for (int x=0;x<num_x;x++)
		for (int y=0;y<num_y;y++){
			Array<vector> sv;
			sv.add(vector((float)(x+1)/(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vector((float) x   /(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vector((float) x   /(float)num_x,(float) y   /(float)num_y,0));
			sv.add(vector((float)(x+1)/(float)num_x,(float) y   /(float)num_y,0));
			Array<int> v;
			v.add(nv+(num_y+1)*(x+1)+y+1);
			v.add(nv+(num_y+1)*(x  )+y+1);
			v.add(nv+(num_y+1)*(x  )+y);
			v.add(nv+(num_y+1)*(x+1)+y);
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
		}
	return &m->Surface.back();
}
