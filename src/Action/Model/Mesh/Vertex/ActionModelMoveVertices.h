/*
 * ActionModelMoveVertices.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEVERTICES_H_
#define ACTIONMODELMOVEVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelMoveVertices: public ActionMultiView
{
public:
	ActionModelMoveVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMoveVertices();
	string name(){	return "ModelMoveVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMOVEVERTICES_H_ */
