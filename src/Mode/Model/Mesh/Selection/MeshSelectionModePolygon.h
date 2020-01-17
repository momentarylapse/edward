/*
 * ModeModelMeshPolygon.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHPOLYGON_H_
#define MODEMODELMESHPOLYGON_H_

#include "MeshSelectionMode.h"

class MeshSelectionModePolygon: public MeshSelectionMode {
public:
	MeshSelectionModePolygon(ModeModelMesh *parent);

	void on_start() override;
	void on_end() override;

	void update_selection() override;
	void update_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
};

#endif /* MODEMODELMESHPOLYGON_H_ */
