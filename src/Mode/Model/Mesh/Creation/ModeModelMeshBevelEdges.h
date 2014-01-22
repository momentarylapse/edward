/*
 * ModeModelMeshBevelEdges.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHBEVELEDGES_H_
#define MODEMODELMESHBEVELEDGES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshBevelEdges: public ModeCreation<DataModel>
{
public:
	ModeModelMeshBevelEdges(ModeBase *_parent);
	virtual ~ModeModelMeshBevelEdges();

	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiView::Window *win);

private:
	float radius;
	float rad_max;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHBEVELEDGES_H_ */
