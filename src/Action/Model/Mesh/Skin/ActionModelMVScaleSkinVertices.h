/*
 * ActionModelMVScaleSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVSCALESKINVERTICES_H_
#define ACTIONMODELMVSCALESKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMVScaleSkinVertices: public ActionMultiView
{
public:
	ActionModelMVScaleSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVScaleSkinVertices();
	string name(){	return "ModelScaleSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMVSCALESKINVERTICES_H_ */
