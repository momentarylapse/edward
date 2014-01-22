/*
 * ModeModelMeshVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHVERTEX_H_
#define MODEMODELMESHVERTEX_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelMeshVertex: public Mode<DataModel>
{
public:
	ModeModelMeshVertex(ModeBase *parent);
	virtual ~ModeModelMeshVertex();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnUpdate(Observable *o);

	virtual void OnDraw();
	virtual void OnDrawWin(MultiView::Window *win);

	void DrawEffects(MultiView::Window *win);
};

extern ModeModelMeshVertex *mode_model_mesh_vertex;

#endif /* MODEMODELMESHVERTEX_H_ */
