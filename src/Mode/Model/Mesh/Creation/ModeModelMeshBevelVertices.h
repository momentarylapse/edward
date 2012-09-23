/*
 * ModeModelMeshBevelVertices.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHBEVELVERTICES_H_
#define MODEMODELMESHBEVELVERTICES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshBevelVertices: public ModeCreation
{
public:
	ModeModelMeshBevelVertices(Mode *_parent);
	virtual ~ModeModelMeshBevelVertices();

	void OnMouseMove();
	void OnKeyDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	float radius;
};

#endif /* MODEMODELMESHBEVELVERTICES_H_ */
