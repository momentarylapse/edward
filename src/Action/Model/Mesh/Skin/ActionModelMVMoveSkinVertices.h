/*
 * ActionModelMVMoveSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMOVESKINVERTICES_H_
#define ACTIONMODELMVMOVESKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMVMoveSkinVertices: public ActionMultiView
{
public:
	ActionModelMVMoveSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVMoveSkinVertices();
	string name(){	return "ModelMoveSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMVMOVESKINVERTICES_H_ */
