/*
 * GeometrySphere.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYSPHERE_H_
#define GEOMETRYSPHERE_H_

#include "PolygonMesh.h"
struct vec3;

class GeometrySphere : public PolygonMesh {
public:
	GeometrySphere(const vec3 &pos, float radius, int num);
	static PolygonMesh create(const vec3 &pos, float radius, int num) {
		return GeometrySphere(pos, radius, num);
	}
};

#endif /* GEOMETRYSPHERE_H_ */
