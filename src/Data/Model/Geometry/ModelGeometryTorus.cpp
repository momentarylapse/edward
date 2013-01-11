/*
 * ModelGeometryTorus.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ModelGeometryTorus.h"
#include "../DataModel.h"

#define _tor_vert(i, j)         ( num_y * ((i) % num_x) + ((j) % num_y))

ModelGeometryTorus::ModelGeometryTorus(const vector &pos, const vector &axis, float radius1, float radius2, int num_x, int num_y)
{
	vector d1 = axis.ortho();
	d1.normalize();
	vector d2 = d1 ^ axis;

	// create vertices
	for (int i=0;i<num_x;i++)
		for (int j=0;j<num_y;j++){
			float wx = (float)i / (float)num_x * 2 * pi;
			float wy = (float)j / (float)num_y * 2 * pi;
			vector e_r = d1 * cos(wx) + d2 * sin(wx);
			vector p = pos + e_r * radius1 + (e_r * cos(wy) + axis * sin(wy)) * radius2;
			AddVertex(p);
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
			AddPolygonSingleTexture(v, sv);
		}
}

