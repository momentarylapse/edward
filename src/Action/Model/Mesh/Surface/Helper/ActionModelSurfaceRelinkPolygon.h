/*
 * ActionModelSurfaceRelinkPolygon.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACERELINKPOLYGON_H_
#define ACTIONMODELSURFACERELINKPOLYGON_H_

#include "../../../../ActionGroup.h"
class DataModel;

class ActionModelSurfaceRelinkPolygon : public ActionGroup
{
public:
	ActionModelSurfaceRelinkPolygon(int _surface, int _polygon, Array<int> &_v, int _new_surface = -1);
	virtual ~ActionModelSurfaceRelinkPolygon(){}
	string name(){	return "ModelSurfaceRelinkPolygon";	}

	void *compose(Data *d);
private:
	int surface, polygon, new_surface;
	Array<int> &v;
};

#endif /* ACTIONMODELSURFACERELINKPOLYGON_H_ */
