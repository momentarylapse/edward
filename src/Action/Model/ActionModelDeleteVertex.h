/*
 * ActionModelDeleteVertex.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEVERTEX_H_
#define ACTIONMODELDELETEVERTEX_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModelDeleteVertex: public Action
{
public:
	ActionModelDeleteVertex(int _vertex);
	virtual ~ActionModelDeleteVertex();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int vertex;
	vector pos;
};

#endif /* ACTIONMODELDELETEVERTEX_H_ */
