/*
 * GeometryTorus.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "GeometryTorus.h"

#define _tor_vert(i, j)         ( num_y * ((i) % num_x) + ((j) % num_y))

GeometryTorus::GeometryTorus(const vec3 &pos, const vec3 &axis, float radius1, float radius2, int num_x, int num_y)
{
	vec3 d1 = axis.ortho();
	d1.normalize();
	vec3 d2 = vec3::cross(d1, axis);

	// create vertices
	for (int i=0;i<num_x;i++)
		for (int j=0;j<num_y;j++){
			float wx = (float)i / (float)num_x * 2 * pi;
			float wy = (float)j / (float)num_y * 2 * pi;
			vec3 e_r = d1 * cos(wx) + d2 * sin(wx);
			vec3 p = pos + e_r * radius1 + (e_r * cos(wy) + axis * sin(wy)) * radius2;
			add_vertex(p);
		}

	// polygons
	for (int i=0;i<num_x;i++)
		for (int j=0;j<num_y;j++){
			Array<int> v;
			v.add(_tor_vert(i+1, j));
			v.add(_tor_vert(i,   j));
			v.add(_tor_vert(i,   j+1));
			v.add(_tor_vert(i+1, j+1));
			Array<vec3> sv;
			sv.add(vec3((float)(i+1) / (float)num_x, (float) j    / (float)num_y, 0));
			sv.add(vec3((float) i    / (float)num_x, (float) j    / (float)num_y, 0));
			sv.add(vec3((float) i    / (float)num_x, (float)(j+1) / (float)num_y, 0));
			sv.add(vec3((float)(i+1) / (float)num_x, (float)(j+1) / (float)num_y, 0));
			add_polygon_single_texture(v, sv);
		}
}

void GeometryTorus::__init__(const vec3& pos, const vec3& axis, float radius1, float radius2, int num_x, int num_y)
{
	new (this) GeometryTorus(pos, axis, radius1, radius2, num_x, num_y);
}
