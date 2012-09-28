/*
 * ModeModelMeshCreatePolygon.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPOLYGON_H_
#define MODEMODELMESHCREATEPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePolygon: public ModeCreation
{
public:
	ModeModelMeshCreatePolygon(Mode *_parent);
	virtual ~ModeModelMeshCreatePolygon();

	void OnStart();
	void OnEnd();

	void OnLeftButtonDown();
	void OnKeyDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}
	Array<int> selection;
};

#endif /* MODEMODELMESHCREATEPOLYGON_H_ */
