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

class ModeModelMeshCreateCylinder: public ModeCreation
{
public:
	ModeModelMeshCreateCylinder(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshCreateCylinder();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnKeyDown();

	void PostDrawWin(int win, irect dest);

	DataModel *data;

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
