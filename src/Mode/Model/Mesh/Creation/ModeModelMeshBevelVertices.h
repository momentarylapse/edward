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

	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(MultiViewWindow *win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	float radius;
	float rad_max;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHBEVELVERTICES_H_ */
