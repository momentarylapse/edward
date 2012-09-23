/*
 * ActionModelCollapseVertices.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELCOLLAPSEVERTICES_H_
#define ACTIONMODELCOLLAPSEVERTICES_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;

class ActionModelCollapseVertices: public ActionGroup
{
public:
	ActionModelCollapseVertices(DataModel *m);
	virtual ~ActionModelCollapseVertices();
	string name(){	return "ModelCollapseVertices";	}

	void CollapseVerticesInSurface(DataModel *m, ModelSurface &s, int surf);
};

#endif /* ACTIONMODELCOLLAPSEVERTICES_H_ */
