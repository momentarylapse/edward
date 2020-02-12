/*
 * ActionModelCollapseEdge.h
 *
 *  Created on: 25.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELCOLLAPSEEDGE_H_
#define ACTIONMODELCOLLAPSEEDGE_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelCollapseEdge : public ActionGroup
{
public:
	ActionModelCollapseEdge(int _edge);
	string name(){	return "ModelCollapseEdge";	}

	void *compose(Data *d);
private:
	int edge;
};

#endif /* ACTIONMODELCOLLAPSEEDGE_H_ */
