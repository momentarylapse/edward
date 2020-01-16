/*
 * ModeModelMeshPaint.h
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_
#define SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_

#include "ModeModelMesh.h"

class ModeModelMeshPaint: public Mode<DataModel>, public Observable {
public:
	ModeModelMeshPaint(ModeBase *parent);
	virtual ~ModeModelMeshPaint();
	void on_start() override;
	void on_end() override;

	//ModelMaterialDialog *dialog;

	void on_selection_change() override;
	void on_set_multi_view() override;
	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;
};

extern ModeModelMeshPaint *mode_model_mesh_paint;

#endif /* SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_ */
