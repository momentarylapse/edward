/*
 * ActionModelMirrorVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMIRRORVERTICES_H_
#define ACTIONMODELMIRRORVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelMirrorVertices: public ActionMultiView
{
public:
	ActionModelMirrorVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMirrorVertices();
	string name(){	return "ModelMirrorVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMIRRORVERTICES_H_ */