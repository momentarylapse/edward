/*
 * ActionModelFlattenVertices.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELFLATTENVERTICES_H_
#define ACTIONMODELFLATTENVERTICES_H_

#include "../../../Action.h"
class DataModel;
class vector;

class ActionModelFlattenVertices : public Action
{
public:
	ActionModelFlattenVertices(DataModel *m);
	virtual ~ActionModelFlattenVertices();
	string name(){	return "ModelFlattenVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

	virtual bool was_trivial();

private:
	Array<int> index;
	Array<vector> old_pos;
};

#endif /* ACTIONMODELFLATTENVERTICES_H_ */
