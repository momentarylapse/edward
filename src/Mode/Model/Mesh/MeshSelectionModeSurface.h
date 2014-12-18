/*
 * ModeModelMeshSurface.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSURFACE_H_
#define MODEMODELMESHSURFACE_H_

#include "MeshSelectionMode.h"

class MeshSelectionModeSurface: public MeshSelectionMode
{
public:
	MeshSelectionModeSurface(ModeModelMesh *parent);

	virtual void onStart();
	virtual void onEnd();

	virtual void updateSelection();
	virtual void updateMultiView();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
};

#endif /* MODEMODELMESHSURFACE_H_ */
