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

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMiddleButtonDown();
	virtual void onMiddleButtonUp();
	virtual void onRightButtonDown();
	virtual void onRightButtonUp();
	virtual void onKeyDown();
	virtual void onKeyUp();
	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o);
	virtual void onUpdateMenu();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);

	void DrawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
};

extern ModeModelMeshEdge *mode_model_mesh_edge;

#endif /* MODEMODELMESHEDGE_H_ */
