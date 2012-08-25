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
	ActionModelCollapseEdge(DataModel *m, int _surface, int _edge);
	virtual ~ActionModelCollapseEdge();
	string name(){	return "ModelCollapseEdge";	}
};

#endif /* ACTIONMODELCOLLAPSEEDGE_H_ */
