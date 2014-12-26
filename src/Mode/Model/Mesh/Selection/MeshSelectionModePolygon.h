/*
 * ModeModelMeshPolygon.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHPOLYGON_H_
#define MODEMODELMESHPOLYGON_H_

#include "MeshSelectionMode.h"

class MeshSelectionModePolygon: public MeshSelectionMode
{
public:
	MeshSelectionModePolygon(ModeModelMesh *parent);

	virtual void onStart();
	virtual void onEnd();

	virtual void updateSelection();
	virtual void updateMultiView();

	virtual void onDrawWin(MultiView::Window *win);
};

#endif /* MODEMODELMESHPOLYGON_H_ */
