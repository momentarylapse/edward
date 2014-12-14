/*
 * ModeModelMeshSurface.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSURFACE_H_
#define MODEMODELMESHSURFACE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelMeshSurface: public Mode<DataModel>
{
public:
	ModeModelMeshSurface(ModeBase *parent);
	virtual ~ModeModelMeshSurface();

	virtual void onStart();
	virtual void onEnd();

	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
};

extern ModeModelMeshSurface *mode_model_mesh_surface;

#endif /* MODEMODELMESHSURFACE_H_ */
