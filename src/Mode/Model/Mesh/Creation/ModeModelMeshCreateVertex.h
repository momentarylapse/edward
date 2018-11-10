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

class ModeModelMeshCreateVertex: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateVertex(ModeBase *parent);
	virtual ~ModeModelMeshCreateVertex();

	void on_left_button_down() override;
};

#endif /* MODEMODELMESHCREATEVERTEX_H_ */
