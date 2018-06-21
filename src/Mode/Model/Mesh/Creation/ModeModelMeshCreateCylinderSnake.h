/*
 * ModeModelMeshCreateCylinderSnake.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDERSNAKE_H_
#define MODEMODELMESHCREATECYLINDERSNAKE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCylinderSnake: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCylinderSnake(ModeBase *parent);

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonUp() override;
	void onCommand(const string& id) override;

	void onDrawWin(MultiView::Window *win) override;

	void updateGeometry();

	void onClose();

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
	bool closed;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDERSNAKE_H_ */
