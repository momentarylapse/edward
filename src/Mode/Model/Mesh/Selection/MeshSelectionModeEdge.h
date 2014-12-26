/*
 * ModeModelMeshEdge.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEDGE_H_
#define MODEMODELMESHEDGE_H_

#include "MeshSelectionMode.h"

class MeshSelectionModeEdge: public MeshSelectionMode
{
public:
	MeshSelectionModeEdge(ModeModelMesh *parent);

	virtual void onStart();
	virtual void onEnd();

	virtual void updateSelection();
	virtual void updateMultiView();

	virtual void onDrawWin(MultiView::Window *win);
};

#endif /* MODEMODELMESHEDGE_H_ */
