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
class ModelGeometry;

class ModeModelMeshCreatePlatonic: public ModeCreation
{
public:
	ModeModelMeshCreatePlatonic(Mode *parent, int type);
	virtual ~ModeModelMeshCreatePlatonic();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(int win);

	void UpdateGeometry();

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	ModelGeometry *geo;
	int type;
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEPLATONIC_H_ */
