/*
 * ActionModelSurfaceAddPolygon.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEADDPOLYGON_H_
#define ACTIONMODELSURFACEADDPOLYGON_H_

#include "../../../../Action.h"
class vector;

class ActionModelSurfaceAddPolygon: public Action
{
public:
	ActionModelSurfaceAddPolygon(int _surface, Array<int> &_v, int _material, Array<vector> &_sv, int _index = -1);
	virtual ~ActionModelSurfaceAddPolygon();
	string name(){	return "ModelSurfaceAddPolygon";	}

	void *execute(Data *d);
	void undo(Data *d);

	// abort: don't undo because exception was thrown without adding a polygon
	void abort(Data *d){}

private:
	int surface;
	int index;
	Array<int> v;
	int material;
	Array<vector> sv;
};

#endif /* ACTIONMODELSURFACEADDPOLYGON_H_ */
