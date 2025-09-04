/*
 * GeometryPlatonic.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "GeometryPlatonic.h"
#include "GeometryCube.h"
#include <lib/math/Box.h>
#include <cmath>

GeometryPlatonic::GeometryPlatonic(const vec3 &pos, float radius, int type) {
	if (type == 4)
		AddTetrahedron(pos, radius);
	else if (type == 6)
		add(GeometryCube::create({pos - vec3(radius, radius, radius), pos + vec3(radius, radius, radius)}, {1, 1, 1}));
	else if (type == 8)
		AddOctahedron(pos, radius);
	else if (type == 12)
		AddDodecahedron(pos, radius);
	else if (type == 20)
		AddIcosahedron(pos, radius);
}


void GeometryPlatonic::AddTetrahedron(const vec3 &pos, float radius) {
	int nv = vertices.num;

	float r = radius / sqrt(3.0f/2);
	add_vertex(pos + vec3( 1, 0, -1/sqrt(2.0f)) * r);
	add_vertex(pos + vec3(-1, 0, -1/sqrt(2.0f)) * r);
	add_vertex(pos + vec3( 0, 1,  1/sqrt(2.0f)) * r);
	add_vertex(pos + vec3( 0,-1,  1/sqrt(2.0f)) * r);
	add_easy(nv, {0, 1, 2});
	add_easy(nv, {1, 0, 3});
	add_easy(nv, {0, 2, 3});
	add_easy(nv, {1, 3, 2});
}

void GeometryPlatonic::AddOctahedron(const vec3 &pos, float radius) {
	int nv = vertices.num;

	float r = radius;
	add_vertex(pos + vec3( 1, 0, 0) * r);
	add_vertex(pos + vec3(-1, 0, 0) * r);
	add_vertex(pos + vec3( 0, 1, 0) * r);
	add_vertex(pos + vec3( 0,-1, 0) * r);
	add_vertex(pos + vec3( 0, 0, 1) * r);
	add_vertex(pos + vec3( 0, 0,-1) * r);
	add_easy(nv, {0, 2, 4});
	add_easy(nv, {4, 2, 1});
	add_easy(nv, {1, 2, 5});
	add_easy(nv, {5, 2, 0});
	add_easy(nv, {4, 3, 0});
	add_easy(nv, {0, 3, 5});
	add_easy(nv, {5, 3, 1});
	add_easy(nv, {1, 3, 4});
}

void GeometryPlatonic::AddDodecahedron(const vec3 &pos, float radius) {
	int nv = vertices.num;

	float phi = (1 + sqrt(5.0f))/2;
	float r = radius / sqrt(3.0f);
	add_vertex(pos + vec3( 1, 1, 1) * r);
	add_vertex(pos + vec3( 1, 1,-1) * r);
	add_vertex(pos + vec3( 1,-1, 1) * r);
	add_vertex(pos + vec3( 1,-1,-1) * r);
	add_vertex(pos + vec3(-1, 1, 1) * r);
	add_vertex(pos + vec3(-1, 1,-1) * r);
	add_vertex(pos + vec3(-1,-1, 1) * r);
	add_vertex(pos + vec3(-1,-1,-1) * r);
	add_vertex(pos + vec3(0, 1/phi, phi) * r);
	add_vertex(pos + vec3(0, 1/phi,-phi) * r);
	add_vertex(pos + vec3(0,-1/phi, phi) * r);
	add_vertex(pos + vec3(0,-1/phi,-phi) * r);
	add_vertex(pos + vec3( 1/phi, phi,0) * r);
	add_vertex(pos + vec3( 1/phi,-phi,0) * r);
	add_vertex(pos + vec3(-1/phi, phi,0) * r);
	add_vertex(pos + vec3(-1/phi,-phi,0) * r);
	add_vertex(pos + vec3( phi,0, 1/phi) * r);
	add_vertex(pos + vec3(-phi,0, 1/phi) * r);
	add_vertex(pos + vec3( phi,0,-1/phi) * r);
	add_vertex(pos + vec3(-phi,0,-1/phi) * r);
	add_easy(nv, {14, 12, 1, 9, 5});
	add_easy(nv, {12, 14, 4, 8, 0});
	add_easy(nv, {13, 15, 7, 11, 3});
	add_easy(nv, {15, 13, 2, 10, 6});
	add_easy(nv, {16, 18, 1, 12, 0});
	add_easy(nv, {18, 16, 2, 13, 3});
	add_easy(nv, {19, 17, 4, 14, 5});
	add_easy(nv, {17, 19, 7, 15, 6});
	add_easy(nv, {9, 11, 7, 19, 5});
	add_easy(nv, {11, 9, 1, 18, 3});
	add_easy(nv, {10, 8, 4, 17, 6});
	add_easy(nv, {8, 10, 2, 16, 0});
}

void GeometryPlatonic::AddIcosahedron(const vec3 &pos, float radius) {
	int nv = vertices.num;

	float phi = (1 + sqrt(5.0f))/2;
	float r = radius / sqrt(phi * sqrt(5.0f));
	add_vertex(pos + vec3( 0, 1, phi) * r);
	add_vertex(pos + vec3( 0, 1,-phi) * r);
	add_vertex(pos + vec3( 0,-1, phi) * r);
	add_vertex(pos + vec3( 0,-1,-phi) * r);
	add_vertex(pos + vec3( 1, phi, 0) * r);
	add_vertex(pos + vec3( 1,-phi, 0) * r);
	add_vertex(pos + vec3(-1, phi, 0) * r);
	add_vertex(pos + vec3(-1,-phi, 0) * r);
	add_vertex(pos + vec3( phi, 0, 1) * r);
	add_vertex(pos + vec3(-phi, 0, 1) * r);
	add_vertex(pos + vec3( phi, 0,-1) * r);
	add_vertex(pos + vec3(-phi, 0,-1) * r);
	add_easy(nv, {0, 2, 8});
	add_easy(nv, {2, 0, 9});
	add_easy(nv, {3, 1,10});
	add_easy(nv, {1, 3,11});
	add_easy(nv, {4, 6, 0});
	add_easy(nv, {6, 4, 1});
	add_easy(nv, {7, 5, 2});
	add_easy(nv, {5, 7, 3});
	add_easy(nv, {8,10, 4});
	add_easy(nv,{10, 8, 5});
	add_easy(nv,{11, 9, 6});
	add_easy(nv, {9,11, 7});
	add_easy(nv, {4, 0, 8});
	add_easy(nv, {0, 6, 9});
	add_easy(nv, {2, 5, 8});
	add_easy(nv, {7, 2, 9});
	add_easy(nv, {3, 7,11});
	add_easy(nv, {5, 3,10});
	add_easy(nv, {6, 1,11});
	add_easy(nv, {1, 4,10});
}
