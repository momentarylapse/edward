/*
 * ModeModelMeshBrushExtrudeVertices.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHBRUSHEXTRUDEVERTICES_H_
#define MODEMODELMESHBRUSHEXTRUDEVERTICES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshBrushExtrudeVertices: public ModeCreation<DataModel>
{
public:
	ModeModelMeshBrushExtrudeVertices(ModeBase *parent);
	virtual ~ModeModelMeshBrushExtrudeVertices();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiViewWindow *win);

private:
	int surface, triangle;
	vector pos;
};

#endif /* MODEMODELMESHBRUSHEXTRUDEVERTICES_H_ */
