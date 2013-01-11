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

class ModeModelMeshCreatePlatonic: public ModeCreation
{
public:
	ModeModelMeshCreatePlatonic(Mode *parent, int type);
	virtual ~ModeModelMeshCreatePlatonic();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	int type;
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEPLATONIC_H_ */
