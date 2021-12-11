/*
 * ModeModelMeshMaterial.h
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MODEMODELMESHMATERIAL_H_
#define SRC_MODE_MODEL_MESH_MODEMODELMESHMATERIAL_H_

#include "ModeModelMesh.h"

class ModelMaterialDialog;

class ModeModelMeshMaterial: public Mode<DataModel> {
public:
	ModeModelMeshMaterial(ModeBase *parent, MultiView::MultiView *mv);
	void on_start() override;
	void on_end() override;

	ModelMaterialDialog *dialog;

	void on_selection_change() override;
	void on_set_multi_view() override;
	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;
};

extern ModeModelMeshMaterial *mode_model_mesh_material;

#endif /* SRC_MODE_MODEL_MESH_MODEMODELMESHMATERIAL_H_ */
