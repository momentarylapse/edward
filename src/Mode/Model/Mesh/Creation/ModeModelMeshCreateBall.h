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

class ModeModelMeshCreateBall: public ModeCreation
{
public:
	ModeModelMeshCreateBall(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshCreateBall();

	void OnMouseMove();
	void OnLeftButtonDown();

	void PostDrawWin(int win, irect dest);

	DataModel *data;

private:
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
