/*
 * ModeModelMeshPaste.h
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshPaste: public ModeCreation<DataModel>
{
public:
	ModeModelMeshPaste(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void update_geometry();

private:
	vector dpos0;
	Geometry *geo;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_ */
