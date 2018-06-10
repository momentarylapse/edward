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

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonDown() override;

	void onDrawWin(MultiView::Window *win) override;

private:
	float radius;
	float rad_max;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHBEVELEDGES_H_ */
