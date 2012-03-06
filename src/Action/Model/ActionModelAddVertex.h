/*
 * ActionAddVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDVERTEX_H_
#define ACTIONMODELADDVERTEX_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModelAddVertex: public Action
{
public:
	ActionModelAddVertex(const vector &_v);
	virtual ~ActionModelAddVertex();

	bool is_atom();
	bool needs_preparation();

	void prepare(Data *d);
	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	vector v;
};

#endif /* ACTIONMODELADDVERTEX_H_ */
