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

class ModeModelMeshCreateCube: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCube(ModeBase *_parent);
	virtual ~ModeModelMeshCreateCube();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiViewWindow *win);

private:
	vector pos, pos2;
	vector length[3];
	bool pos_chosen;
	bool pos2_chosen;
};

#endif /* MODEMODELMESHCREATECUBE_H_ */
