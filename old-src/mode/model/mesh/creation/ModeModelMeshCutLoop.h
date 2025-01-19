/*
 * ModeModelMeshCutLoop.h
 *
 *  Created on: Feb 16, 2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHCUTLOOP_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHCUTLOOP_H_



#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelMesh;

class ModeModelMeshCutLoop: public ModeCreation<ModeModelMesh, DataModel> {
public:
	ModeModelMeshCutLoop(ModeModelMesh *parent);

	void on_start() override;

	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;


private:

	bool valid_loop = false;
	Array<int> edges;
	Array<int> edge_directions;
	Array<int> polygons;
	Array<int> polygon_sides;
	Array<vec3> pos;

	bool find_loop(int edge, const vec3 &v);
	void calculate_edge_positions(float f);
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHCUTLOOP_H_ */
