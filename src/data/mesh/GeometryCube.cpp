/*
 * GeometryCube.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "GeometryCube.h"

GeometryCube::GeometryCube(const vec3 &_pos, const vec3 &dv1, const vec3 &dv2, const vec3 &dv3, int num_1, int num_2, int num_3)
{
	vec3 pos = _pos;
	vec3 dv[3];
	int num[3];
	dv[0] = dv1;
	dv[1] = dv2;
	dv[2] = dv3;
	num[0] = num_1;
	num[1] = num_2;
	num[2] = num_3;

	// check orientation
	if (vec3::dot(vec3::cross(dv[0], dv[1]), dv[2]) < 0){
		pos += dv[2];
		dv[2] = - dv[2];
	}

	int nv = 0;
	for (int f=0;f<6;f++){
	//	nv = Vertex.num;
		int nz = f % 3;
		int z = f / 3;
		int ny = (nz == 1) ? 2 : 1;
		int nx = 3 - ny - nz;

		// create new vertices
		for (int x=0;x<=num[nx];x++)
			for (int y=0;y<=num[ny];y++){
				int xx = ((nz == 2) ^ (z == 0)) ? x : num[nx] - x;
				vec3 dp = dv[nx] * (float)xx / (float)num[nx] + dv[ny] * (float)y / (float)num[ny] + dv[nz] * z;
				add_vertex(pos + dp);
			}
		// create new triangles
		for (int x=0;x<num[nx];x++)
			for (int y=0;y<num[ny];y++){
				Array<int> v;
				v.add(nv + (num[ny]+1)* x   +y+1);
				v.add(nv + (num[ny]+1)* x   +y);
				v.add(nv + (num[ny]+1)*(x+1)+y);
				v.add(nv + (num[ny]+1)*(x+1)+y+1);
				Array<vec3> sv;
				sv.add(vec3((float) x   /(float)num[nx],(float)(y+1)/(float)num[ny],0));
				sv.add(vec3((float) x   /(float)num[nx],(float) y   /(float)num[ny],0));
				sv.add(vec3((float)(x+1)/(float)num[nx],(float) y   /(float)num[ny],0));
				sv.add(vec3((float)(x+1)/(float)num[nx],(float)(y+1)/(float)num[ny],0));
				add_polygon_single_texture(v, sv);
			}
		nv += (num[nx] + 1) * (num[ny] + 1);
	}

	float epsilon = min(min(dv[0].length() / (float)num[0], dv[1].length() / (float)num[1]), dv[2].length() / (float)num[2]) * 0.01f;

	// weld together
	weld(epsilon);

}

void GeometryCube::__init__(const vec3& pos, const vec3& dv1, const vec3& dv2, const vec3& dv3, int num_1, int num_2, int num_3)
{
	new (this) GeometryCube(pos, dv1, dv2, dv3, num_1, num_2, num_3);
}
