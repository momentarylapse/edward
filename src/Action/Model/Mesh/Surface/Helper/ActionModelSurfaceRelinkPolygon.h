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
	ActionModelSurfaceRelinkPolygon(int _polygon, Array<int> &_v);
	virtual ~ActionModelSurfaceRelinkPolygon(){}
	string name(){	return "ModelSurfaceRelinkPolygon";	}

	void *compose(Data *d);
private:
	int polygon;
	Array<int> &v;
};

#endif /* ACTIONMODELSURFACERELINKPOLYGON_H_ */
