/*
 * ActionModelNearifyVertices.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELNEARIFYVERTICES_H_
#define ACTIONMODELNEARIFYVERTICES_H_

#include "../../../Action.h"
class DataModel;
class vec3;

class ActionModelNearifyVertices : public Action
{
public:
	ActionModelNearifyVertices(DataModel *m);
	virtual ~ActionModelNearifyVertices();
	string name(){	return "ModelNearifyVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

	virtual bool was_trivial();

private:
	Array<int> index;
	Array<vec3> old_pos;
};

#endif /* ACTIONMODELNEARIFYVERTICES_H_ */
