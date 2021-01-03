/*
 * ModeModelMeshEdge.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEDGE_H_
#define MODEMODELMESHEDGE_H_

#include "MeshSelectionMode.h"

class MeshSelectionModeEdge: public MeshSelectionMode {
public:
	MeshSelectionModeEdge(ModeModelMesh *parent);

	void on_start() override;
	void on_end() override;

	void on_update_selection() override;
	void on_view_stage_change() override;
	void update_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
};

#endif /* MODEMODELMESHEDGE_H_ */
