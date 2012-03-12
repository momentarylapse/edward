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

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);

	void PostDraw();
	void PostDrawWin(int win, irect dest);

	DataModel *data;

private:
	vector pos, pos2, length;
	float radius;
	bool pos_chosen, pos2_chosen;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
