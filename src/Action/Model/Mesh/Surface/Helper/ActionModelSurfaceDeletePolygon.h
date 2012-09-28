/*
 * ActionModelSurfaceDeletePolygon.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEDELETEPOLYGON_H_
#define ACTIONMODELSURFACEDELETEPOLYGON_H_

#include "../../../../Action.h"
class vector;

class ActionModelSurfaceDeletePolygon: public Action
{
public:
	ActionModelSurfaceDeletePolygon(int _surface, int _index);
	virtual ~ActionModelSurfaceDeletePolygon();
	string name(){	return "ModelSurfaceDeletePolygon";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface, index;
	int material;
	Array<int> vertex;
	Array<vector> skin;
};

#endif /* ACTIONMODELSURFACEDELETEPOLYGON_H_ */
