/*
 * Platonic.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "Platonic.h"
#include "Cube.h"
#include <lib/math/Box.h>
#include <cmath>


namespace polymesh {
	PolygonMesh create_platonic(const vec3 &pos, float radius, int type) {
		if (type == 4)
			return create_tetrahedron(pos, radius);
		if (type == 6)
			return create_cube({pos - vec3(radius, radius, radius), pos + vec3(radius, radius, radius)}, {1, 1, 1});
		if (type == 8)
			return create_octahedron(pos, radius);
		if (type == 12)
			return create_dodecahedron(pos, radius);
		if (type == 20)
			return create_icosahedron(pos, radius);
		return {};
	}

	PolygonMesh create_tetrahedron(const vec3& pos, float radius) {
		PolygonMesh m;

		const float r = radius / sqrtf(3.0f/2);
		m.add_vertex(pos + vec3( 1, 0, -1/sqrtf(2.0f)) * r);
		m.add_vertex(pos + vec3(-1, 0, -1/sqrtf(2.0f)) * r);
		m.add_vertex(pos + vec3( 0, 1,  1/sqrtf(2.0f)) * r);
		m.add_vertex(pos + vec3( 0,-1,  1/sqrtf(2.0f)) * r);
		m.add_easy(0, {0, 1, 2});
		m.add_easy(0, {1, 0, 3});
		m.add_easy(0, {0, 2, 3});
		m.add_easy(0, {1, 3, 2});
		return m;
	}

	PolygonMesh create_octahedron(const vec3& pos, float radius) {
		PolygonMesh m;
		const float r = radius;
		m.add_vertex(pos + vec3( 1, 0, 0) * r);
		m.add_vertex(pos + vec3(-1, 0, 0) * r);
		m.add_vertex(pos + vec3( 0, 1, 0) * r);
		m.add_vertex(pos + vec3( 0,-1, 0) * r);
		m.add_vertex(pos + vec3( 0, 0, 1) * r);
		m.add_vertex(pos + vec3( 0, 0,-1) * r);
		m.add_easy(0, {0, 2, 4});
		m.add_easy(0, {4, 2, 1});
		m.add_easy(0, {1, 2, 5});
		m.add_easy(0, {5, 2, 0});
		m.add_easy(0, {4, 3, 0});
		m.add_easy(0, {0, 3, 5});
		m.add_easy(0, {5, 3, 1});
		m.add_easy(0, {1, 3, 4});
		return m;
	}

	PolygonMesh create_dodecahedron(const vec3& pos, float radius) {
		PolygonMesh m;

		float phi = (1 + sqrtf(5.0f))/2;
		float r = radius / sqrtf(3.0f);
		m.add_vertex(pos + vec3( 1, 1, 1) * r);
		m.add_vertex(pos + vec3( 1, 1,-1) * r);
		m.add_vertex(pos + vec3( 1,-1, 1) * r);
		m.add_vertex(pos + vec3( 1,-1,-1) * r);
		m.add_vertex(pos + vec3(-1, 1, 1) * r);
		m.add_vertex(pos + vec3(-1, 1,-1) * r);
		m.add_vertex(pos + vec3(-1,-1, 1) * r);
		m.add_vertex(pos + vec3(-1,-1,-1) * r);
		m.add_vertex(pos + vec3(0, 1/phi, phi) * r);
		m.add_vertex(pos + vec3(0, 1/phi,-phi) * r);
		m.add_vertex(pos + vec3(0,-1/phi, phi) * r);
		m.add_vertex(pos + vec3(0,-1/phi,-phi) * r);
		m.add_vertex(pos + vec3( 1/phi, phi,0) * r);
		m.add_vertex(pos + vec3( 1/phi,-phi,0) * r);
		m.add_vertex(pos + vec3(-1/phi, phi,0) * r);
		m.add_vertex(pos + vec3(-1/phi,-phi,0) * r);
		m.add_vertex(pos + vec3( phi,0, 1/phi) * r);
		m.add_vertex(pos + vec3(-phi,0, 1/phi) * r);
		m.add_vertex(pos + vec3( phi,0,-1/phi) * r);
		m.add_vertex(pos + vec3(-phi,0,-1/phi) * r);
		m.add_easy(0, {14, 12, 1, 9, 5});
		m.add_easy(0, {12, 14, 4, 8, 0});
		m.add_easy(0, {13, 15, 7, 11, 3});
		m.add_easy(0, {15, 13, 2, 10, 6});
		m.add_easy(0, {16, 18, 1, 12, 0});
		m.add_easy(0, {18, 16, 2, 13, 3});
		m.add_easy(0, {19, 17, 4, 14, 5});
		m.add_easy(0, {17, 19, 7, 15, 6});
		m.add_easy(0, {9, 11, 7, 19, 5});
		m.add_easy(0, {11, 9, 1, 18, 3});
		m.add_easy(0, {10, 8, 4, 17, 6});
		m.add_easy(0, {8, 10, 2, 16, 0});
		return m;
	}

	PolygonMesh create_icosahedron(const vec3& pos, float radius) {
		PolygonMesh m;

		float phi = (1 + sqrtf(5.0f))/2;
		float r = radius / sqrtf(phi * sqrtf(5.0f));
		m.add_vertex(pos + vec3( 0, 1, phi) * r);
		m.add_vertex(pos + vec3( 0, 1,-phi) * r);
		m.add_vertex(pos + vec3( 0,-1, phi) * r);
		m.add_vertex(pos + vec3( 0,-1,-phi) * r);
		m.add_vertex(pos + vec3( 1, phi, 0) * r);
		m.add_vertex(pos + vec3( 1,-phi, 0) * r);
		m.add_vertex(pos + vec3(-1, phi, 0) * r);
		m.add_vertex(pos + vec3(-1,-phi, 0) * r);
		m.add_vertex(pos + vec3( phi, 0, 1) * r);
		m.add_vertex(pos + vec3(-phi, 0, 1) * r);
		m.add_vertex(pos + vec3( phi, 0,-1) * r);
		m.add_vertex(pos + vec3(-phi, 0,-1) * r);
		m.add_easy(0, {0, 2, 8});
		m.add_easy(0, {2, 0, 9});
		m.add_easy(0, {3, 1,10});
		m.add_easy(0, {1, 3,11});
		m.add_easy(0, {4, 6, 0});
		m.add_easy(0, {6, 4, 1});
		m.add_easy(0, {7, 5, 2});
		m.add_easy(0, {5, 7, 3});
		m.add_easy(0, {8,10, 4});
		m.add_easy(0,{10, 8, 5});
		m.add_easy(0,{11, 9, 6});
		m.add_easy(0, {9,11, 7});
		m.add_easy(0, {4, 0, 8});
		m.add_easy(0, {0, 6, 9});
		m.add_easy(0, {2, 5, 8});
		m.add_easy(0, {7, 2, 9});
		m.add_easy(0, {3, 7,11});
		m.add_easy(0, {5, 3,10});
		m.add_easy(0, {6, 1,11});
		m.add_easy(0, {1, 4,10});
		return m;
	}
}
