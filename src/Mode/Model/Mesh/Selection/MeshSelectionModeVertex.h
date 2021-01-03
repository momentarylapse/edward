/*
 * ModeModelMeshVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHVERTEX_H_
#define MODEMODELMESHVERTEX_H_

#include "MeshSelectionMode.h"


class MeshSelectionModeVertex: public MeshSelectionMode {
public:
	MeshSelectionModeVertex(ModeModelMesh *_parent);

	void on_start() override;
	void on_end() override;

	void on_update_selection() override;
	void on_view_stage_change() override;
	void update_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
};

#endif /* MODEMODELMESHVERTEX_H_ */
