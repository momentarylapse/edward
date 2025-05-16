/*
 * GeometrySphere.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "GeometrySphere.h"
#include <lib/math/mat4.h>

GeometrySphere::GeometrySphere(const vec3 &pos, float radius, int num)
{
	int nv = 0;

	// sphere = "blown up cube"
	for (int f=0;f<6;f++){
	//	nv = Vertex.num;
		mat4 mat;
		if (f==0)	mat = mat4::ID;
		if (f==1)	mat = mat4::rotation_x( pi/2);
		if (f==2)	mat = mat4::rotation_x(-pi/2);
		if (f==3)	mat = mat4::rotation_y( pi/2);
		if (f==4)	mat = mat4::rotation_x( pi);
		if (f==5)	mat = mat4::rotation_y(-pi/2);
		// create new vertices
		for (int x=0;x<num+1;x++)
			for (int y=0;y<num+1;y++){
				vec3 dp=vec3(float(x*2-num),float(y*2-num),float(num));
				dp.normalize();
				dp = mat * dp;
				add_vertex(pos + radius * dp);
			}
		// create new triangles
		for (int x=0;x<num;x++)
			for (int y=0;y<num;y++){
				Array<int> v;
				v.add(nv + (num+1)* x   +y+1);
				v.add(nv + (num+1)* x   +y);
				v.add(nv + (num+1)*(x+1)+y);
				v.add(nv + (num+1)*(x+1)+y+1);
				Array<vec3> sv;
				sv.add(vec3((float) x   /(float)num,(float)(y+1)/(float)num,0));
				sv.add(vec3((float) x   /(float)num,(float) y   /(float)num,0));
				sv.add(vec3((float)(x+1)/(float)num,(float) y   /(float)num,0));
				sv.add(vec3((float)(x+1)/(float)num,(float)(y+1)/(float)num,0));
				add_polygon_single_texture(v, sv);
			}
		nv += (num + 1) * (num + 1);
	}
	weld(radius / num * 0.01f);
}
