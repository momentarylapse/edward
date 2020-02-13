/*
 * ModeModelMeshBevelEdges.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHBEVELEDGES_H_
#define MODEMODELMESHBEVELEDGES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelSelection.h"

class ModeModelMeshBevelEdges: public ModeCreation<DataModel> {
public:
	ModeModelMeshBevelEdges(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_down() override;

	void on_draw_win(MultiView::Window *win) override;

private:
	float radius;
	float rad_max;
	ModelSelection selection;
};

#endif /* MODEMODELMESHBEVELEDGES_H_ */
