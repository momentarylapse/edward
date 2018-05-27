/*
 * ModeModelMeshCreateBall.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEBALL_H_
#define MODEMODELMESHCREATEBALL_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateBall: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateBall(ModeBase *_parent);
	virtual ~ModeModelMeshCreateBall();

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonUp() override;

	void onDrawWin(MultiView::Window *win) override;

	void updateGeometry();
	void onTypeBall();
	void onTypeSphere();
	void onTypePhysical();

private:
	vector pos;
	float radius;
	bool pos_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
