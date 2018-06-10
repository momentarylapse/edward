/*
 * ModeModelMeshCreatePlane.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLANE_H_
#define MODEMODELMESHCREATEPLANE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePlane: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePlane(ModeBase *parent);

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonUp() override;

	void onDrawWin(MultiView::Window *win) override;

	void onClose();

private:
	vector pos, pos2;
	vector length[2];
	bool pos_chosen;
	bool invert;
};

#endif /* MODEMODELMESHCREATEPLANE_H_ */
