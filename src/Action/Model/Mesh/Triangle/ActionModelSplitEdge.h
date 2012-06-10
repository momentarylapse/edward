/*
 * ActionModelSplitEdge.h
 *
 *  Created on: 09.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSPLITEDGE_H_
#define ACTIONMODELSPLITEDGE_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelSplitEdge : public ActionGroup
{
public:
	ActionModelSplitEdge(DataModel *m, int _surface, int _edge, const vector &_pos);
	virtual ~ActionModelSplitEdge();
	string name(){	return "ModelSplitEdge";	}
};

#endif /* ACTIONMODELSPLITEDGE_H_ */
