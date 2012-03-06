/*
 * ActionAddTriangle.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLE_H_
#define ACTIONMODELADDTRIANGLE_H_

#include "../Action.h"
#include "../ActionAtom.h"
#include "../../lib/types/types.h"

class ActionModelAddTriangle: public ActionAtom
{
public:
	ActionModelAddTriangle(int _a, int _b, int _c, const vector &_sva, const vector &_svb, const vector &_svc);
	virtual ~ActionModelAddTriangle();

	bool needs_preparation();

	void prepare(Data *d);
	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int a, b, c;
	vector sv[3];
};

#endif /* ACTIONMODELADDTRIANGLE_H_ */
