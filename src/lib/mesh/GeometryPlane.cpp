/*
 * GeometryPlane.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "GeometryPlane.h"
#include <assert.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>

GeometryPlane::GeometryPlane(const vec3 &pos, const vec3 &dv1, const vec3 &dv2, int num_x, int num_y)
{
	assert(num_x * num_y > 0);

	/// vertices
	vec3 dx = dv1 / num_x;
	vec3 dy = dv2 / num_y;
	for (int x=0;x<num_x+1;x++)
		for (int y=0;y<num_y+1;y++)
			add_vertex(pos + dx*(float)x + dy*(float)y);

	// triangles
	for (int x=0;x<num_x;x++)
		for (int y=0;y<num_y;y++){
			Array<vec3> sv;
			sv.add(vec3((float)(x+1)/(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vec3((float) x   /(float)num_x,(float)(y+1)/(float)num_y,0));
			sv.add(vec3((float) x   /(float)num_x,(float) y   /(float)num_y,0));
			sv.add(vec3((float)(x+1)/(float)num_x,(float) y   /(float)num_y,0));
			Array<int> v;
			v.add((num_y+1)*(x+1)+y+1);
			v.add((num_y+1)*(x  )+y+1);
			v.add((num_y+1)*(x  )+y);
			v.add((num_y+1)*(x+1)+y);
			add_polygon_single_texture(v, sv);
		}
}

PolygonMesh GeometryPlane::create(const rect& r, const ivec2& slices) {
	return GeometryPlane({r.x1, r.y1, 0}, {r.width(), 0, 0}, {0, r.height(), 0}, slices.i, slices.j);
}

