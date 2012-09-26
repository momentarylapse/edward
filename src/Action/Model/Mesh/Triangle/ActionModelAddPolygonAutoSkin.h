/*
 * ActionModelAddPolygonAutoSkin.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPOLYGONAUTOSKIN_H_
#define ACTIONMODELADDPOLYGONAUTOSKIN_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelAddPolygonAutoSkin: public ActionGroup
{
public:
	ActionModelAddPolygonAutoSkin(Array<int> &_vertex);
	virtual ~ActionModelAddPolygonAutoSkin(){}
	string name(){	return "ModelAddPolygonAutoSkin";	}

	void *compose(Data *d);

private:
	Array<int> vertex;
};

#endif /* ACTIONMODELADDPOLYGONAUTOSKIN_H_ */
