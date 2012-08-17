/*
 * ActionModelMoveSkinVertices.h
 *
 *  Created on: 15.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVESKINVERTICES_H_
#define ACTIONMODELMOVESKINVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class ActionModelMoveSkinVertices: public ActionMultiView
{
public:
	ActionModelMoveSkinVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMoveSkinVertices();
	string name(){	return "ModelMoveSkinVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material, texture_level;
	Array<int> surface;
};

#endif /* ACTIONMODELMOVESKINVERTICES_H_ */
