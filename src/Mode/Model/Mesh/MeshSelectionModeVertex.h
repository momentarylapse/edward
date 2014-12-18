/*
 * ModeModelMeshVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHVERTEX_H_
#define MODEMODELMESHVERTEX_H_

#include "MeshSelectionMode.h"


class MeshSelectionModeVertex: public MeshSelectionMode
{
public:
	MeshSelectionModeVertex(ModeModelMesh *_parent);

	virtual void onStart();
	virtual void onEnd();

	virtual void updateSelection();
	virtual void updateMultiView();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
};

#endif /* MODEMODELMESHVERTEX_H_ */
