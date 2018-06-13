/*
 * ModeModelMeshCreatePolygon.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPOLYGON_H_
#define MODEMODELMESHCREATEPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePolygon: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePolygon(ModeBase *parent);

	void onStart() override;
	void onEnd() override;

	void onLeftButtonDown() override;
	void onCommand(const string &id) override;

	void onDrawWin(MultiView::Window *win) override;

	Array<int> selection;
};

#endif /* MODEMODELMESHCREATEPOLYGON_H_ */
