/*
 * ModeModelMeshCreateCylinder.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDER_H_
#define MODEMODELMESHCREATECYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCylinder: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCylinder(ModeBase *parent);
	virtual ~ModeModelMeshCreateCylinder();

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonUp() override;

	void onDrawWin(MultiView::Window *win) override;

	void onTypeVisible();
	void onTypePhysical();

	void updateGeometry();

	void onClose();

private:
	Array<vector> pos;
	vector length;
	float radius;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
