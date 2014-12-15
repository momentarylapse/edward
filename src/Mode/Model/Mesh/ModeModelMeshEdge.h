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

	virtual void onStart();
	virtual void onEnd();

	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDrawWin(MultiView::Window *win);

	void drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
};

extern ModeModelMeshEdge *mode_model_mesh_edge;

#endif /* MODEMODELMESHEDGE_H_ */
