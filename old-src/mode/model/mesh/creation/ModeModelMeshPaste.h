/*
 * ModeModelMeshPaste.h
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class Geometry;
class ModeModelMesh;

class ModeModelMeshPaste: public ModeCreation<ModeModelMesh, DataModel>
{
public:
	ModeModelMeshPaste(ModeModelMesh *_parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void update_geometry();

private:
	vec3 dpos0;
	owned<Geometry> geo;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_ */
