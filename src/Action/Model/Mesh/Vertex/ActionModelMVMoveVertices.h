/*
 * ActionModelMVMoveVertices.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMOVEVERTICES_H_
#define ACTIONMODELMVMOVEVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelMVMoveVertices: public ActionMultiView
{
public:
	ActionModelMVMoveVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVMoveVertices();
	string name(){	return "ModelMoveVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMVMOVEVERTICES_H_ */
