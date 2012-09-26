/*
 * ActionModelSplitEdge.h
 *
 *  Created on: 09.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSPLITEDGE_H_
#define ACTIONMODELSPLITEDGE_H_

#include "../../../ActionGroup.h"
class DataModel;
class vector;

class ActionModelSplitEdge : public ActionGroup
{
public:
	ActionModelSplitEdge(int _surface, int _edge, float _factor);
	virtual ~ActionModelSplitEdge(){}
	string name(){	return "ModelSplitEdge";	}

	void *compose(Data *d);
private:
	int surface;
	int edge;
	float factor;
};

#endif /* ACTIONMODELSPLITEDGE_H_ */
