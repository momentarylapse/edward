/*
 * ModeModelMeshEdge.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEDGE_H_
#define MODEMODELMESHEDGE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelMeshEdge: public Mode<DataModel>
{
public:
	ModeModelMeshEdge(ModeBase *parent);
	virtual ~ModeModelMeshEdge();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnKeyDown();
	virtual void OnKeyUp();
	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDraw();
	virtual void OnDrawWin(MultiViewWindow *win);
};

extern ModeModelMeshEdge *mode_model_mesh_edge;

#endif /* MODEMODELMESHEDGE_H_ */
