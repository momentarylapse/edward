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

class Mode;
class DataModel;

class ModeModelMeshSurface: public Mode
{
public:
	ModeModelMeshSurface(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshSurface();

	void OnStart();
	void OnEnd();

	void OnUpdate(Observable *o);

	void OnDraw();
	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}
};

extern ModeModelMeshSurface *mode_model_mesh_surface;

#endif /* MODEMODELMESHSURFACE_H_ */
