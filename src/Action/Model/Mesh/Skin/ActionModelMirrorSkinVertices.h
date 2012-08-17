/*
 * ActionModelMirrorSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMIRRORSKINVERTICES_H_
#define ACTIONMODELMIRRORSKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMirrorSkinVertices: public ActionMultiView
{
public:
	ActionModelMirrorSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMirrorSkinVertices();
	string name(){	return "ModelMirrorSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMIRRORSKINVERTICES_H_ */
