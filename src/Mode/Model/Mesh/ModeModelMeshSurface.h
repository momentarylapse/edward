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

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);

	DataModel *data;
};

extern ModeModelMeshSurface *mode_model_mesh_surface;

#endif /* MODEMODELMESHSURFACE_H_ */
