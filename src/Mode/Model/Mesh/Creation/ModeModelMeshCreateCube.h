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
	ModeModelMeshCreateCube(Mode *_parent);
	virtual ~ModeModelMeshCreateCube();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(MultiViewWindow *win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	vector pos, pos2;
	vector length[3];
	bool pos_chosen;
	bool pos2_chosen;
};

#endif /* MODEMODELMESHCREATECUBE_H_ */
