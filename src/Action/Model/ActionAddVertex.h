/*
 * ActionAddVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTIONADDVERTEX_H_
#define ACTIONADDVERTEX_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionAddVertex: public Action
{
public:
	ActionAddVertex(const vector &_v, int _skin);
	virtual ~ActionAddVertex();

	void execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	vector v;
	int skin;
};

#endif /* ACTIONADDVERTEX_H_ */
