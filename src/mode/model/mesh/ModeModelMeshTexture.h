/*
 * ModeModelMeshTexture.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHTEXTURE_H_
#define MODEMODELMESHTEXTURE_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
class ModelMaterialDialog;
class ModelSkinVertexDummy;
class ModeModelMesh;

class ModeModelMeshTexture: public Mode<ModeModelMesh, DataModel> {
public:
	ModeModelMeshTexture(ModeModelMesh *parent, MultiView::MultiView *mv);

	class State : public obs::Node<VirtualBase> {
	public:
		obs::source out_texture_level_changed{this, "texture-level-changed"};
		~State(){}
	} state;

	void on_start() override;
	void on_end() override;

	void on_data_skin_change();
	void on_data_change();

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;
	void on_selection_change() override;

	// for MultiView actions
	void getSelectedSkinVertices(Array<int> &tria, Array<int> &index);


	void fetchData();
	int getNumSelected();

	ModelMaterialDialog *dialog;

	Array<ModelSkinVertexDummy> skin_vertex;

	int current_texture_level;
	void set_current_texture_level(int level);
};

#endif /* MODEMODELMESHTEXTURE_H_ */
