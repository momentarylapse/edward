/*
 * ActionModelScaleSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSCALESKINVERTICES_H_
#define ACTIONMODELSCALESKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelScaleSkinVertices: public ActionMultiView
{
public:
	ActionModelScaleSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelScaleSkinVertices();
	string name(){	return "ModelScaleSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELSCALESKINVERTICES_H_ */
