/*
 * ModeModelMeshCreateCube.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECUBE_H_
#define MODEMODELMESHCREATECUBE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCube: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCube(ModeBase *_parent);
	virtual ~ModeModelMeshCreateCube();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonUp();

	virtual void onDrawWin(MultiView::Window *win);

	void onClose();

	void updateGeometry();

private:
	vector pos, pos2;
	vector length[3];
	bool pos_chosen;
	bool pos2_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECUBE_H_ */
