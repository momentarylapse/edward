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

class ModeModelMeshCreateCube: public ModeCreation
{
public:
	ModeModelMeshCreateCube(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshCreateCube();

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
};

#endif /* MODEMODELMESHCREATECUBE_H_ */
