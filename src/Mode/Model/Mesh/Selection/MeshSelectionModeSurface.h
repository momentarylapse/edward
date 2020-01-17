/*
 * ModeModelMeshSurface.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSURFACE_H_
#define MODEMODELMESHSURFACE_H_

#include "MeshSelectionMode.h"

class MeshSelectionModeSurface: public MeshSelectionMode {
public:
	MeshSelectionModeSurface(ModeModelMesh *parent);

	void on_start() override;
	void on_end() override;

	void update_selection() override;
	void update_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
};

#endif /* MODEMODELMESHSURFACE_H_ */
