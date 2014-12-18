/*
 * MeshSelectionMode.h
 *
 *  Created on: 18.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_
#define SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_

#include "ModeModelMesh.h"

class MeshSelectionMode
{
public:
	MeshSelectionMode(ModeModelMesh *_parent);
	virtual ~MeshSelectionMode(){}

	virtual void onStart(){}
	virtual void onEnd(){}

	virtual void updateSelection(){}
	virtual void updateMultiView(){}

	virtual void onDrawWin(MultiView::Window *win){}

	bool isActive();

	ModeModelMesh *parent;
	DataModel *data;
	MultiView::MultiView *multi_view;
};

#endif /* SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_ */
