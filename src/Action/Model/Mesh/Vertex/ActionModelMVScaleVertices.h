/*
 * ActionModelMVScaleVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVSCALEVERTICES_H_
#define ACTIONMODELMVSCALEVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelMVScaleVertices: public ActionMultiView
{
public:
	ActionModelMVScaleVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVScaleVertices();
	string name(){	return "ModelScaleVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
};

#endif /* ACTIONMODELMVSCALEVERTICES_H_ */
