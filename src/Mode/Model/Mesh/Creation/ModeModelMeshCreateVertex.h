/*
 * ModeModelMeshCreateVertex.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEVERTEX_H_
#define MODEMODELMESHCREATEVERTEX_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreateVertex: public ModeCreation
{
public:
	ModeModelMeshCreateVertex(Mode *_parent);
	virtual ~ModeModelMeshCreateVertex();

	void OnLeftButtonDown();

	DataModel *data;
	Data *GetData(){	return data;	}
};

#endif /* MODEMODELMESHCREATEVERTEX_H_ */
