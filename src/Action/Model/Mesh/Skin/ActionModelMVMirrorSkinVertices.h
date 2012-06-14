/*
 * ActionModelMVMirrorSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMIRRORSKINVERTICES_H_
#define ACTIONMODELMVMIRRORSKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMVMirrorSkinVertices: public ActionMultiView
{
public:
	ActionModelMVMirrorSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVMirrorSkinVertices();
	string name(){	return "ModelMirrorSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMVMIRRORSKINVERTICES_H_ */
