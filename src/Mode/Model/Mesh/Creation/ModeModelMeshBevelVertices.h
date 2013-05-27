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

class ModeModelMeshBevelVertices: public ModeCreation<DataModel>
{
public:
	ModeModelMeshBevelVertices(ModeBase *_parent);
	virtual ~ModeModelMeshBevelVertices();

	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiViewWindow *win);

private:
	float radius;
	float rad_max;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHBEVELVERTICES_H_ */
