/*
 * ActionModelScaleVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSCALEVERTICES_H_
#define ACTIONMODELSCALEVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelScaleVertices: public ActionMultiView
{
public:
	ActionModelScaleVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelScaleVertices();
	string name(){	return "ModelScaleVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
};

#endif /* ACTIONMODELSCALEVERTICES_H_ */
