/*
 * ActionModel__DeleteVertex.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__DELETEVERTEX_H_
#define ACTIONMODEL__DELETEVERTEX_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModel__DeleteVertex: public Action
{
public:
	ActionModel__DeleteVertex(int _vertex);
	virtual ~ActionModel__DeleteVertex();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vector pos;
	int bone, normal_mode;
	Array<vector> move;
};

#endif /* ACTIONMODEL__DELETEVERTEX_H_ */
