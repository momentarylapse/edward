/*
 * GeometryTorusKnot.h
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYTORUSKNOT_H_
#define GEOMETRYTORUSKNOT_H_

#include "PolygonMesh.h"

class GeometryTorusKnot : public PolygonMesh {
public:
	GeometryTorusKnot(const vec3 &pos, const vec3 &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges);
	static PolygonMesh create(const vec3 &pos, const vec3 &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges) {
		return GeometryTorusKnot(pos, axis, radius1, radius2, radius3, turns, twists, rings, edges);
	}
};

#endif /* GEOMETRYTORUSKNOT_H_ */
