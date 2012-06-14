/*
 * ActionModelMVRotateSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVROTATESKINVERTICES_H_
#define ACTIONMODELMVROTATESKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMVRotateSkinVertices: public ActionMultiView
{
public:
	ActionModelMVRotateSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVRotateSkinVertices();
	string name(){	return "ModelRotateSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMVROTATESKINVERTICES_H_ */
