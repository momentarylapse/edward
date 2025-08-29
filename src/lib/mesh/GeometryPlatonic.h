/*
 * GeometryPlatonic.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYPLATONIC_H_
#define GEOMETRYPLATONIC_H_

#include "PolygonMesh.h"
struct vec3;

class GeometryPlatonic : public PolygonMesh {
public:
	GeometryPlatonic(const vec3 &pos, float radius, int type);
	static PolygonMesh create(const vec3 &pos, float radius, int type) {
		return GeometryPlatonic(pos, radius, type);
	}
private:
	void AddTetrahedron(const vec3 &pos, float radius);
	void AddOctahedron(const vec3 &pos, float radius);
	void AddDodecahedron(const vec3 &pos, float radius);
	void AddIcosahedron(const vec3 &pos, float radius);
};

#endif /* GEOMETRYPLATONIC_H_ */
