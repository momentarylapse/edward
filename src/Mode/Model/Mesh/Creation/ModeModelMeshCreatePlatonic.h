/*
 * ModeModelMeshCreatePlatonic.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLATONIC_H_
#define MODEMODELMESHCREATEPLATONIC_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
class Geometry;

class ModeModelMeshCreatePlatonic: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePlatonic(ModeBase *parent, int type);
	virtual ~ModeModelMeshCreatePlatonic();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiView::Window *win);

	void UpdateGeometry();

private:
	Geometry *geo;
	int type;
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEPLATONIC_H_ */
