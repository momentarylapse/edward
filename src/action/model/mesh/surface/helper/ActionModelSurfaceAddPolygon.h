/*
 * ActionModelSurfaceAddPolygon.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEADDPOLYGON_H_
#define ACTIONMODELSURFACEADDPOLYGON_H_

#include "../../../../Action.h"
class vec3;

class ActionModelSurfaceAddPolygon: public Action {
public:
	ActionModelSurfaceAddPolygon(const Array<int> &_v, int _material, const Array<vec3> &_sv, int _index = -1);
	string name() { return "ModelSurfaceAddPolygon"; }

	void *execute(Data *d);
	void undo(Data *d);

	// abort: don't undo because exception was thrown without adding a polygon
	void abort(Data *d) {}

private:
	int index;
	Array<int> v;
	int material;
	Array<vec3> sv;
};

#endif /* ACTIONMODELSURFACEADDPOLYGON_H_ */
