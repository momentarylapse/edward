/*
 * ModelGeometryPlane.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "ModelGeometryPlane.h"
#include "../DataModel.h"
#include <assert.h>

ModelGeometryPlane::ModelGeometryPlane(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y)
{
	assert(num_x * num_y > 0);

	/// vertices
	vector dx = dv1 / num_x;
	vector dy = dv2 / num_y;
	for (int x=0;x<num_x+1;x++)
		for (int y=0;y<num_y+1;y++)
			AddVertex(pos + dx*(float)x + dy*(float)y);

	// triangles
	for (int x=0;x<num_x;x++)
		for (int y=0;y<num_y;y++){
			Array<vector> sv;
			sv.add(vector((float)(x+1)/(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vector((float) x   /(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vector((float) x   /(float)num_x,(float) y   /(float)num_y,0));
			sv.add(vector((float)(x+1)/(float)num_x,(float) y   /(float)num_y,0));
			Array<int> v;
			v.add((num_y+1)*(x+1)+y+1);
			v.add((num_y+1)*(x  )+y+1);
			v.add((num_y+1)*(x  )+y);
			v.add((num_y+1)*(x+1)+y);
			AddPolygonSingleTexture(v, sv);
		}
}

void ModelGeometryPlane::__init__(const vector& pos, const vector& dv1, const vector& dv2, int num_x, int num_y)
{
	new (this) ModelGeometryPlane(pos, dv1, dv2, num_x, num_y);
}
